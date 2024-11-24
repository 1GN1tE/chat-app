#include "db_pool.hpp"

ConnectionPool::ConnectionPool(const std::string &dbName, int poolSize)
    : dbName(dbName), poolSize(poolSize)
{
    initializePool();
}

ConnectionPool::~ConnectionPool()
{
    while (!connections.empty())
    {
        sqlite3 *conn = connections.front();
        sqlite3_close(conn);
        connections.pop();
    }
}

void ConnectionPool::initializePool()
{
    for (int i = 0; i < poolSize; ++i)
    {
        sqlite3 *conn;
        if (sqlite3_open(dbName.c_str(), &conn) == SQLITE_OK)
        {
            connections.push(conn);
        }
        else
        {
            throw std::runtime_error("Failed to open database connection: " + std::string(sqlite3_errmsg(conn)));
        }
    }
}

sqlite3 *ConnectionPool::acquireConnection()
{
    std::unique_lock<std::mutex> lock(mutex);
    condition.wait(lock, [this]
                   { return !connections.empty(); });
    sqlite3 *conn = connections.front();
    connections.pop();
    return conn;
}

void ConnectionPool::releaseConnection(sqlite3 *conn)
{
    std::lock_guard<std::mutex> lock(mutex);
    connections.push(conn);
    condition.notify_one();
}
