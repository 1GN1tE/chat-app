#pragma once

#include <sqlite3.h>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <string>
#include <stdexcept>
#include <iostream>

class ConnectionPool
{
public:
    // Constructor that initializes the connection pool with a database name and size
    ConnectionPool(const std::string &dbName, int poolSize);

    // Destructor that cleans up all database connections in the pool
    ~ConnectionPool();

    // Acquire a connection from the pool
    sqlite3 *acquireConnection();

    // Release a connection back to the pool
    void releaseConnection(sqlite3 *conn);

private:
    std::string dbName;
    int poolSize;
    std::queue<sqlite3 *> connections;
    std::mutex mutex;
    std::condition_variable condition;

    // Initialize the pool with database connections
    void initializePool();
};
