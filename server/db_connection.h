#ifndef DB_CONNECTION_H
#define DB_CONNECTION_H

#include <pqxx/pqxx>
#include <memory>
#include <iostream>

class Database {
    public:
        Database();
        ~Database();
        bool isConnected();
        pqxx::result exec(const std::string& query);  
        pqxx::connection* getConnection();
    
    private:
        std::unique_ptr<pqxx::connection> conn;
    };

#endif // DB_CONNECTION_H
