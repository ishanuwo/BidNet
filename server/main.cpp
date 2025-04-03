#include "crow.h"
#include "crow/middlewares/cors.h"
#include "db_connection.h"
#include <pqxx/pqxx>

// Global database instance
Database db;

int main() {
    if (!db.isConnected()) {
        std::cerr << "Failed to connect to database. Exiting..." << std::endl;
        return 1;
    }

    // Enable CORS
    crow::App<crow::CORSHandler> app;

    // Customize CORS
    auto& cors = app.get_middleware<crow::CORSHandler>();

    // Set up CORS for all routes (you can specify more headers as needed)
    cors
      .global()
        .headers("Content-Type", "Authorization")  // Allowed headers
        .methods("POST"_method, "GET"_method, "PUT"_method, "DELETE"_method, "OPTIONS"_method)
        .max_age(5)  // Cache preflight request for 5 seconds
      .prefix("/")
        .origin("*");

    // Register routes
    CROW_ROUTE(app, "/register").methods("POST"_method)([](const crow::request& req) {
        auto body = crow::json::load(req.body);
        if (!body) {
            return crow::response(400, "Invalid JSON");
        }

        std::string username = body["username"].s();
        std::string email = body["email"].s();
        std::string password = body["password"].s();

        try {
            pqxx::work txn(*db.getConnection());
            std::string query = "INSERT INTO users (username, email, password_hash) "
                                "VALUES (" + txn.quote(username) + ", " + txn.quote(email) + ", " + txn.quote(password) + ");";
            txn.exec(query);
            txn.commit();
            return crow::response(201, "User registered successfully");
        } catch (const std::exception &e) {
            return crow::response(500, "Error: " + std::string(e.what()));
        }
    });

    CROW_ROUTE(app, "/login").methods("POST"_method)([](const crow::request& req) {
        auto body = crow::json::load(req.body);
        if (!body) {
            return crow::response(400, "Invalid JSON");
        }
    
        std::string username = body["username"].s();
        std::string password = body["password"].s();
    
        try {
            pqxx::work txn(*db.getConnection());
            // Modify the query to also fetch the user_id along with the password
            std::string query = "SELECT id, password_hash FROM users WHERE username = " + txn.quote(username) + ";";
            pqxx::result result = txn.exec(query);
    
            if (result.empty()) {
                return crow::response(400, "User not found");
            }
    
            std::string stored_password = result[0][1].c_str();
            if (stored_password == password) {
                int id = result[0][0].as<int>(); // Retrieve the user_id from the result
                crow::json::wvalue res;
                res["message"] = "Login successful";
                res["id"] = id; // Return the user_id in the response
                return crow::response(200, res);
            } else {
                crow::json::wvalue res;
                res["message"] = "Invalid password";
                return crow::response(400, res);
            }
        } catch (const std::exception &e) {
            return crow::response(500, "Error: " + std::string(e.what()));
        }
    });
    
    CROW_ROUTE(app, "/edit_user").methods("PUT"_method)([](const crow::request& req) {
        auto body = crow::json::load(req.body);
        if (!body) {
            return crow::response(400, "Invalid JSON");
        }

        std::string username = body["username"].s();
        std::string new_email = body["new_email"].s();
        std::string new_password = body["new_password"].s();

        try {
            pqxx::work txn(*db.getConnection());
            std::string query = "UPDATE users SET email = " + txn.quote(new_email) +
                                ", password_hash = " + txn.quote(new_password) +
                                " WHERE username = " + txn.quote(username) + ";";
            txn.exec(query);
            txn.commit();
            return crow::response(200, "User information updated successfully");
        } catch (const std::exception &e) {
            return crow::response(500, "Error: " + std::string(e.what()));
        }
    });

    CROW_ROUTE(app, "/create_auction").methods("POST"_method)([](const crow::request& req) {
        auto body = crow::json::load(req.body);
        if (!body) {
            return crow::response(400, "Invalid JSON");
        }

        // Extract auction data from JSON
        int userId = body["user_id"].i(); 
        // std::cout << "User ID: " << userId << std::endl; // Debugging line
        std::string name = body["name"].s();
        std::string description = body["description"].s();
        double startingPrice = body["starting_price"].d();
        int duration = body["duration"].i(); // Duration in seconds
        // convert duration in hours to end time
        std::string bidEndTimeQuery = "NOW() + INTERVAL '" + std::to_string(duration) + " seconds'";

        try {
            pqxx::work txn(*db.getConnection());
            std::string query =
            "INSERT INTO items (user_id, name, description, starting_price, current_price, bid_end_time) "
            "VALUES (" + txn.quote(userId) + ", " + txn.quote(name) + ", " + txn.quote(description) + ", " +
            txn.quote(startingPrice) + ", " + txn.quote(startingPrice) + ", " + bidEndTimeQuery + ");";
        
            txn.exec(query);
            txn.commit();

            crow::json::wvalue res;
            res["status"] = "success";
            return crow::response(201, res);
        } catch (const std::exception &e) {
            return crow::response(500, "Error: " + std::string(e.what()));
        }
    });

    CROW_ROUTE(app, "/get_all_items").methods("GET"_method)([](const crow::request& req) {
        try {
            pqxx::work txn(*db.getConnection());

            // Query to get all items, including the 'id' field
            std::string query = "SELECT id, user_id, name, description, starting_price, bid_end_time, status FROM items";
            pqxx::result res = txn.exec(query);

            crow::json::wvalue response_data;
            std::vector<crow::json::wvalue> items;  // Create a vector to hold item JSONs

            // Loop through the result set and add each item to the response
            for (const auto& row : res) {
                crow::json::wvalue item;
                item["id"] = row["id"].as<int>();  // Add 'id' field
                item["user_id"] = row["user_id"].as<int>();
                item["name"] = row["name"].as<std::string>();
                item["description"] = row["description"].as<std::string>();
                item["starting_price"] = row["starting_price"].as<double>();
                item["bid_end_time"] = row["bid_end_time"].as<std::string>(); // Format as needed
                item["status"] = row["status"].as<std::string>(); // Assuming it's an ISO 8601 string
                items.push_back(item); // Add the item to the vector
            }

            // Assign the vector of items to the response_data
            response_data["items"] = std::move(items);

            txn.commit();

            // Return the response with the list of items
            return crow::response(200, response_data);
        } catch (const std::exception &e) {
            return crow::response(500, "Error: " + std::string(e.what()));
        }
    });

    CROW_ROUTE(app, "/get_item_details/<string>").methods("GET"_method)([](const crow::request& req, const std::string& id) {
        try {
            pqxx::work txn(*db.getConnection());

            // Query to get all details of an item by 'id'
            std::string query = "SELECT id, user_id, name, description, starting_price, current_price, bid_end_time, status  FROM items WHERE id = $1";
            pqxx::result res = txn.exec_params(query, id); // Execute the query with the 'id' parameter

            // Check if the result is empty (i.e., item not found)
            if (res.empty()) {
                return crow::response(404, "Item not found");
            }

            // Prepare the response with all item details
            crow::json::wvalue response_data;
            const auto& row = res[0]; // We expect only one row since id is unique
            response_data["id"] = row["id"].as<int>();
            response_data["user_id"] = row["user_id"].as<int>();
            response_data["name"] = row["name"].as<std::string>();
            response_data["description"] = row["description"].as<std::string>();
            response_data["starting_price"] = row["starting_price"].as<double>();
            response_data["current_price"] = row["current_price"].as<double>();
            response_data["bid_end_time"] = row["bid_end_time"].as<std::string>(); // Assuming it's an ISO 8601 string
            response_data["status"] = row["status"].as<std::string>(); // Assuming it's an ISO 8601 string
            txn.commit();

            // Return the response with the item details
            return crow::response(200, response_data);
        } catch (const std::exception& e) {
            return crow::response(500, "Error: " + std::string(e.what()));
        }
    });

    CROW_ROUTE(app, "/get_auction_details/<string>").methods("GET"_method)([](const crow::request& req, const std::string& id) {
        try {
            pqxx::work txn(*db.getConnection());

            // Query to fetch auction details by 'id'
            std::string query = "SELECT id, user_id, name, description, starting_price, current_price, bid_end_time, status  FROM items WHERE id = $1";
            pqxx::result res = txn.exec_params(query, id); // Execute the query with the id parameter

            if (res.empty()) {
                return crow::response(404, "Auction not found");
            }

            crow::json::wvalue response_data;
            const auto& row = res[0]; // We expect only one row, since id is unique
            response_data["id"] = row["id"].as<int>();
            response_data["user_id"] = row["user_id"].as<int>();
            response_data["name"] = row["name"].as<std::string>();
            response_data["description"] = row["description"].as<std::string>();
            response_data["starting_price"] = row["starting_price"].as<double>();
            response_data["current_price"] = row["current_price"].as<double>();
            response_data["bid_end_time"] = row["bid_end_time"].as<std::string>(); // Assuming it's an ISO 8601 string
            response_data["status"] = row["status"].as<std::string>(); // Assuming it's an ISO 8601 string
            txn.commit();

            // Return the auction details as JSON
            return crow::response(200, response_data);

        } catch (const std::exception& e) {
            // Handle any errors that occur
            return crow::response(500, "Error: " + std::string(e.what()));
        }
    });

    CROW_ROUTE(app, "/place_bid").methods("POST"_method)([](const crow::request& req) {
        try {
            // Parse the JSON body
            auto bid_data = crow::json::load(req.body);
            if (!bid_data) {
                return crow::response(400, "Invalid JSON");
            }

            // Extract the bid data from the request
            int user_id = bid_data["user_id"].i();
            int seller_id = bid_data["seller_id"].i();
            int item_id = bid_data["item_id"].i();
            double bid_amount = bid_data["bid_amount"].d();

            // Check if the user_id, item_id, and bid_amount are valid
            if (user_id <= 0 || item_id <= 0 || bid_amount <= 0) {
                return crow::response(400, "Invalid bid data");
            }

            pqxx::work txn(*db.getConnection());

            // Get the current price of the item
            std::string query = "SELECT current_price FROM items WHERE id = $1";
            pqxx::result res = txn.exec_params(query, item_id);

            if (res.empty()) {
                return crow::response(404, "Item not found");
            }

            double current_price = res[0]["current_price"].as<double>();

            // Check if the bid is higher than the current price
            if (bid_amount <= current_price) {
                crow::json::wvalue response_data;
                response_data["message"] = "Bid must be higher than the current price";
                return crow::response(400,  response_data);
            }

            // Insert the new bid into the bids table
            std::string insert_bid_query = "INSERT INTO bids (user_id, item_id, bid_amount, seller_id) VALUES ($1, $2, $3, $4)";
            txn.exec_params(insert_bid_query, user_id, item_id, bid_amount, seller_id);

            // Update the current price of the item to the new bid
            std::string update_price_query = "UPDATE items SET current_price = $1 WHERE id = $2";
            txn.exec_params(update_price_query, bid_amount, item_id);

            txn.commit();

            // Return a success response
            crow::json::wvalue response_data;
            response_data["message"] = "Bid placed successfully";
            response_data["new_bid_amount"] = bid_amount;

            return crow::response(200, response_data);

        } catch (const std::exception& e) {
            crow::json::wvalue response_data;
            response_data["message"] = "Error: " + std::string(e.what());
            return crow::response(400,  response_data);
            return crow::response(500, response_data);
        }
    });

    CROW_ROUTE(app, "/get_transactions_for_buyer/<int>").methods("GET"_method)([](const crow::request& req, const int buyer_id) {
        try {
            pqxx::work txn(*db.getConnection());
    
            // Query to fetch transactions where the buyer_id matches the parameter
            std::string query = "SELECT id, item_id, buyer_id, seller_id, final_price, transaction_time FROM transactions WHERE buyer_id = $1";
            pqxx::result res = txn.exec_params(query, buyer_id); // Execute the query with the buyer_id parameter
    
            if (res.empty()) {
                return crow::response(404, "No transactions found for the buyer");
            }
    
            crow::json::wvalue response_data;
            std::vector<crow::json::wvalue> items;
            for (const auto& row : res) {
                crow::json::wvalue transaction;
                transaction["id"] = row["id"].as<int>();
                transaction["item_id"] = row["item_id"].as<int>();
                transaction["buyer_id"] = row["buyer_id"].as<int>();
                transaction["seller_id"] = row["seller_id"].as<int>();
                transaction["final_price"] = row["final_price"].as<double>();
                transaction["transaction_time"] = row["transaction_time"].as<std::string>();
    
                items.push_back(transaction); // Add the transaction to the response array
            }
            response_data["items"] = std::move(items);
            txn.commit();
    
            // Return the list of transactions for the buyer
            return crow::response(200, response_data);
    
        } catch (const std::exception& e) {
            // Handle any errors that occur
            return crow::response(500, "Error: " + std::string(e.what()));
        }
    });

    CROW_ROUTE(app, "/get_transactions_for_seller/<int>").methods("GET"_method)([](const crow::request& req, const int seller_id) {
            try {
                pqxx::work txn(*db.getConnection());

                // Query to fetch transactions where the seller_id matches the parameter
                std::string query = "SELECT id, item_id, buyer_id, seller_id, final_price, transaction_time FROM transactions WHERE seller_id = $1";
                pqxx::result res = txn.exec_params(query, seller_id); // Execute the query with the seller_id parameter

                if (res.empty()) {
                    return crow::response(404, "No transactions found for the seller");
                }

                crow::json::wvalue response_data;
                std::vector<crow::json::wvalue> items;
                for (const auto& row : res) {
                    crow::json::wvalue transaction;
                    transaction["id"] = row["id"].as<int>();
                    transaction["item_id"] = row["item_id"].as<int>();
                    transaction["buyer_id"] = row["buyer_id"].as<int>();
                    transaction["seller_id"] = row["seller_id"].as<int>();
                    transaction["final_price"] = row["final_price"].as<double>();
                    transaction["transaction_time"] = row["transaction_time"].as<std::string>();

                    items.push_back(transaction); // Add the transaction to the response array
                }
                response_data["items"] = std::move(items);
                txn.commit();

                // Return the list of transactions for the seller
                return crow::response(200, response_data);

            } catch (const std::exception& e) {
                // Handle any errors that occur
                return crow::response(500, "Error: " + std::string(e.what()));
            }
        });

    


    
        CROW_ROUTE(app, "/")([](){
        return "Hit from Crow!";
    });

    CROW_ROUTE(app, "/complete_transaction/<int>").methods("POST"_method)([](const crow::request& req, const int item_id) {
        try {
            pqxx::work txn(*db.getConnection());
    
            // Query to get the highest bid for the given item_id
            std::string query = "SELECT user_id, bid_amount, seller_id FROM bids WHERE item_id = $1 ORDER BY bid_amount DESC LIMIT 1";
            pqxx::result res = txn.exec_params(query, item_id);
    
            if (res.empty()) {
                return crow::response(404, "No bids found for the item");
            }
    
            // Get buyer_id and highest bid amount
            int buyer_id = res[0]["user_id"].as<int>();
            double bid_amount = res[0]["bid_amount"].as<double>();
            int seller_id = res[0]["seller_id"].as<int>();
    
            // Create a new transaction entry in the transactions table
            std::string transaction_query = "INSERT INTO transactions (item_id, buyer_id, seller_id, final_price, transaction_time) "
                                           "VALUES ($1, $2, $3, $4, NOW()) RETURNING id";
            pqxx::result transaction_res = txn.exec_params(transaction_query, item_id, buyer_id, seller_id, bid_amount);
    
            if (transaction_res.empty()) {
                return crow::response(500, "Failed to create transaction");
            }
    
            int transaction_id = transaction_res[0]["id"].as<int>();

            std::string update_item_query = "UPDATE items SET status = 'sold' WHERE id = $1";
            txn.exec_params(update_item_query, item_id);
    
            txn.commit();
    
            // Return the details of the completed transaction
            crow::json::wvalue response_data;
            response_data["transaction_id"] = transaction_id;
            response_data["item_id"] = item_id;
            response_data["buyer_id"] = buyer_id;
            response_data["seller_id"] = seller_id;
            response_data["final_price"] = bid_amount;
            response_data["transaction_time"] = "current time";
    
            return crow::response(200, response_data);
    
        } catch (const std::exception& e) {
            // Handle any errors that occur
            return crow::response(500, "Error: " + std::string(e.what()));
        }
    });    

    app.port(8080).run();

    return 0;
}
