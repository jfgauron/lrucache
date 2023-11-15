# Problem

Design a cache with these criteria:

1 - Simplicity. Integration needs to be dead simple.

2 - Resilient to network failures or crashes.

3 - Near real-time replication of data across Geolocation. Writes need to be in real-time.

4 - Data consistency across regions

5 - Locality of reference, data should almost always be available from the closest region

6 - Flexible Schema

7 - Cache can expire


# Solution


The proposed solution is to use the Raft consensus algorithm in order to manage the cache data. Here is how each criteria is met given that solution:

## Simplicity

3 lines of code are all that is needed to run the cache:

```
auto config = lrucache::cache_config::from_file(argv[1]);
lrucache::server server(config);

server.run();
```

Server settings are configured with a [cache.INI file](https://github.com/jfgauron/lrucache/blob/master/cache1.ini).

The lrucache library also has a client included to communicate with the cluster.

## Resilient

The implementation of the Raft algorithm allows the cluster to survive the termination of any nodes in the cluster without issue.

## Real-time replication

All information is replicated accross every nodes in the cluster. The nodes also have a local cache in order to store writes instantly without requiring consensus from the rest of the cluster.

## Data consistency

All data in the cluster is eventually consistent.

## Locality of reference (geo location)

The client is aware of each nodes in the cluster and using a geo locating service such as [geoapify](https://apidocs.geoapify.com/docs/ip-geolocation), it will always communicate with the closest node if available.

## Flexible schema

The cache supports storing any kind of data

## Cache can expire

The LRU cache implementation use expiry buckets in order to offer expiration operations in O(1). The cache implementation can be found there: https://github.com/jfgauron/lrucache/tree/master/libs/lrucache/src/cache.
