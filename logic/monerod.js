const monerodService = require('services/monerod.js');
const MonerodError = require('models/errors.js').MonerodError;

async function getBlockCount() {
  const blockCount = await monerodService.getBlockCount();

  console.log(blockCount)
  return {blockCount: blockCount.result};
}

async function getConnectionsCount() {
  const peerInfo = await monerodService.getPeerInfo();

  var outBoundConnections = 0;
  var inBoundConnections = 0;
  var clearnetConnections = 0;
  var torConnections = 0;
  var i2pConnections = 0;

  peerInfo.result.forEach(function(peer) {
    if (peer.inbound === false) {
      outBoundConnections++;
    } else {
      inBoundConnections++;
    }

    if (peer.network === "onion") {
      torConnections++;
    } else if (peer.network === "i2p") {
      i2pConnections++;
    } else {
      // ipv4 and ipv6 are clearnet
      clearnetConnections++;
    }   
  });

  const connections = {
    total: inBoundConnections + outBoundConnections,
    inbound: inBoundConnections,
    outbound: outBoundConnections,
    clearnet: clearnetConnections,
    tor: torConnections,
    i2p: i2pConnections
  };

  return connections;
}

async function getStatus() {
  try {
    console.log('getting status')
    const info = await monerodService.getBlockChainInfo();

    console.log('info', info);

    return {operational: true};
  } catch (error) {
    if (error instanceof MonerodError) {
      return {operational: false};
    }

    throw error;
  }
}

// Return the max synced header for all connected peers or -1 if no data is available.
async function getMaxSyncHeader() {
  const peerInfo = (await monerodService.getPeerInfo()).result;

  if (peerInfo.length === 0) {
    return -1;
  }

  const maxPeer = peerInfo.reduce(function(prev, current) {
    return prev.syncedHeaders > current.syncedHeaders ? prev : current;
  });

  return maxPeer.syncedHeaders;
}

async function getMempoolInfo() {
  return await monerodService.getMempoolInfo();
}

async function getLocalSyncInfo() {
  const info = await monerodService.getBlockChainInfo();

  var blockChainInfo = info.result;
  var chain = blockChainInfo.chain;
  var blockCount = blockChainInfo.blocks;
  var headerCount = blockChainInfo.headers;
  var percent = blockChainInfo.verificationprogress;
  var pruned = blockChainInfo.pruned;
  var pruneTargetSize = blockChainInfo.pruneTargetSize;

  return {
    chain,
    percent,
    currentBlock: blockCount,
    headerCount: headerCount, // eslint-disable-line object-shorthand,
    pruned,
    pruneTargetSize
  };
}

async function getSyncStatus() {
  const maxPeerHeader = await getMaxSyncHeader();
  const localSyncInfo = await getLocalSyncInfo();

  if (maxPeerHeader > localSyncInfo.headerCount) {
    localSyncInfo.headerCount = maxPeerHeader;
  }

  return localSyncInfo;
}

// TODO - consider using getNetworkInfo for info on proxy for ipv4 and ipv6
async function getVersion() {
  const thing = await monerodService.getVersion();


  // Remove all non-digits or decimals.
  const version = unformattedVersion.replace(/[^\d.]/g, '');

  return {version: version}; // eslint-disable-line object-shorthand
}

async function getTransaction(txid) {
  const transactionObj = await monerodService.getTransaction(txid);

  return {
    txid,
    timestamp: transactionObj.result.time,
    confirmations: transactionObj.result.confirmations,
    blockhash: transactionObj.result.blockhash,
    size: transactionObj.result.size,
    input: transactionObj.result.vin.txid,
    utxo: transactionObj.result.vout,
    rawtx: transactionObj.result.hex
  };
}

async function getNetworkInfo() {
  const networkInfo = await monerodService.getNetworkInfo();

  return networkInfo.result; // eslint-disable-line object-shorthand
}

async function getBlock(hash) {
  const blockObj = await monerodService.getBlock(hash);

  return {
    block: hash,
    confirmations: blockObj.result.confirmations,
    size: blockObj.result.size,
    height: blockObj.result.height,
    blocktime: blockObj.result.time,
    prevblock: blockObj.result.previousblockhash,
    nextblock: blockObj.result.nextblockhash,
    transactions: blockObj.result.tx
  };
}

const memoizedGetFormattedBlock = () => {
  const cache = {};

  return async blockHeight => {
    // cache cleanup
    // 6 blocks/hr * 24 hrs/day * 7 days = 1008 blocks over 7 days
    // plus some wiggle room in case weird difficulty adjustment or period of faster blocks
    const CACHE_LIMIT = 1100;
    while(Object.keys(cache).length > CACHE_LIMIT) {
      const cacheItemToDelete = Object.keys(cache)[0];
      delete cache[cacheItemToDelete];
    }
    
    if (blockHeight in cache) {
      return cache[blockHeight];
    } else {
      let blockHash;
      try {
        ({result: blockHash} = await monerodService.getBlockHash(blockHeight));
      } catch (error) {
        if (error instanceof MonerodError) {
          return error;
        }
        throw error;
      }

      const {result: block} = await monerodService.getBlock(blockHash);

      cache[blockHeight] = {
        hash: block.hash,
        height: block.height,
        numTransactions: block.tx.length,
        confirmations: block.confirmations,
        time: block.time,
        size: block.size,
        previousblockhash: block.previousblockhash
      };

      return cache[blockHeight];
    }
  };
};

const initializedMemoizedGetFormattedBlock = memoizedGetFormattedBlock();


async function getBlockRangeTransactionChunks(fromHeight, toHeight, blocksPerChunk) {
  const {blocks} = await getBlocks(fromHeight, toHeight);
  const chunks = [];
  blocks.forEach((block, index) => {
    const chunkIndex = Math.floor(index / blocksPerChunk);
    if (!chunks[chunkIndex]) {
      chunks[chunkIndex] = {
        time: block.time,
        numTransactions: 0,
      };
    }
    chunks[chunkIndex].numTransactions += block.numTransactions;
  });

  return chunks;
}

async function getBlocks(fromHeight, toHeight) {
  const blocks = [];

  // loop from 'to height' till 'from Height'
  for (let currentHeight = toHeight; currentHeight >= fromHeight; currentHeight--) {
    // terminate loop if we reach the genesis block
    if (currentHeight === 0) {
      break;
    }

    try {
      const formattedBlock = await initializedMemoizedGetFormattedBlock(currentHeight);
      blocks.push(formattedBlock);
    } catch(e) {
      console.error('Error memoizing formatted blocks')
    }
  }

  return {blocks};
}

async function getBlockHash(height) {
  const getBlockHashObj = await monerodService.getBlockHash(height);

  return {
    hash: getBlockHashObj.result
  };
}

async function nodeStatusDump() {
  const blockchainInfo = await monerodService.getBlockChainInfo();
  const networkInfo = await monerodService.getNetworkInfo();
  const mempoolInfo = await monerodService.getMempoolInfo();
  const miningInfo = await monerodService.getMiningInfo();

  return {
    blockchain_info: blockchainInfo.result,
    network_info: networkInfo.result,
    mempool: mempoolInfo.result,
    mining_info: miningInfo.result
  };
}

async function nodeStatusSummary() {
  const blockchainInfo = await monerodService.getBlockChainInfo();
  const networkInfo = await monerodService.getNetworkInfo();
  const mempoolInfo = await monerodService.getMempoolInfo();
  const miningInfo = await monerodService.getMiningInfo();

  return {
    difficulty: blockchainInfo.result.difficulty,
    size: blockchainInfo.result.sizeOnDisk,
    mempool: mempoolInfo.result.bytes,
    connections: networkInfo.result.connections,
    networkhashps: miningInfo.result.networkhashps
  };
}

async function stop() {
  const stopResponse = await monerodService.stop();
  return {stopResponse};
}

module.exports = {
  getBlockHash,
  getTransaction,
  getBlock,
  getBlockCount,
  getBlocks,
  getBlockRangeTransactionChunks,
  getConnectionsCount,
  getNetworkInfo,
  getMempoolInfo,
  getStatus,
  getSyncStatus,
  getVersion,
  nodeStatusDump,
  nodeStatusSummary,
  stop
};
