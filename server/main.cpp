#include "crow.h"

int main() {
    crow::SimpleApp app;

    // Define a basic route
    CROW_ROUTE(app, "/")([](){
        return "Hello, Crow!";
    });

    app.port(8080).multithreaded().run();
}
