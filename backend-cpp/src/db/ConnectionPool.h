#pragma once

#include <pqxx/pqxx>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace weather {

class ConnectionPool {
public:
    explicit ConnectionPool(const std::string& connStr, int size = 8);
    ~ConnectionPool();

    // RAII handle – returns the connection to the pool on destruction
    class DbConn {
    public:
        DbConn(ConnectionPool& pool, std::unique_ptr<pqxx::connection> conn);
        ~DbConn();

        pqxx::connection& get()       { return *conn_; }
        pqxx::connection* operator->() { return  conn_.get(); }

        // Non-copyable, movable
        DbConn(DbConn&&) = default;
        DbConn& operator=(DbConn&&) = default;
        DbConn(const DbConn&) = delete;
        DbConn& operator=(const DbConn&) = delete;

    private:
        ConnectionPool* pool_;
        std::unique_ptr<pqxx::connection> conn_;
    };

    DbConn acquire();

private:
    void release(std::unique_ptr<pqxx::connection> conn);

    std::string connStr_;
    std::vector<std::unique_ptr<pqxx::connection>> pool_;
    std::mutex              mutex_;
    std::condition_variable cv_;

    friend class DbConn;
};

// Module-level singleton (initialised in main)
void          initPool(const std::string& connStr, int size = 8);
ConnectionPool& getPool();

} // namespace weather
