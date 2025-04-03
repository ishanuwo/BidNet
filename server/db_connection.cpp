#include "db_connection.h"

#define DB_IP "34.133.14.189"
#define DB_PORT "5432"
#define DB_NAME "bidnetdb"
#define DB_USER "postgres"
#define DB_PASSWORD "bidnetuser"

Database::Database() {
    try {
        std::string conn_str = 
            "host=" DB_IP 
            " port=" DB_PORT 
            " dbname=" DB_NAME 
            " user=" DB_USER 
            " password=" DB_PASSWORD;

        conn = std::make_unique<pqxx::connection>(conn_str);

        if (conn->is_open()) {
            std::cout << "Connected to database: " << conn->dbname() << std::endl;
        } else {
            std::cerr << "Failed to connect to database" << std::endl;
        }
    } catch (const std::exception &e) {
        std::cerr << "Database connection error: " << e.what() << std::endl;
    }
}

Database::~Database() {
    if (conn && conn->is_open()) {
        conn->close();
        std::cout << "Database connection closed" << std::endl;
    }
}

bool Database::isConnected() {
    return conn && conn->is_open();
}
pqxx::connection* Database::getConnection() {
    return conn.get();
}
pqxx::result Database::exec(const std::string& query) {
    pqxx::work txn(*conn);
    return txn.exec(query);
}
