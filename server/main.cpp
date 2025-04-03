#include "crow.h"
#include "db_connection.h"

int main() {
    Database db; // Attempt to connect to DB on startup

    if (!db.isConnected()) {
        std::cerr << "Failed to connect to database. Exiting..." << std::endl;
        return 1;
    }

    crow::SimpleApp app;

    // Define a basic route
    CROW_ROUTE(app, "/")([](){
        return "Hello, Crow!";
    });

    app.port(8080).multithreaded().run();
}
