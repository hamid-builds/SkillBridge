#include "managers/SQLiteOrderRepository.h"
#include "managers/DatabaseManager.h"
#include "core/Exceptions.h"
#include "sqlite3.h"

#include <string>

using namespace std;

void SQLiteOrderRepository::throwPrepareError(const string& sql) const {
    sqlite3* db = DatabaseManager::getInstance().getConnection();
    string msg = "Failed to prepare statement. SQL: " + sql +
        " | Error: " + sqlite3_errmsg(db);
    throw DatabaseException(msg);
}

Order SQLiteOrderRepository::buildOrderFromRow(sqlite3_stmt* stmt) const {
    
    int orderID = sqlite3_column_int(stmt, 0);
    int gigID = sqlite3_column_int(stmt, 1);
    int buyerID = sqlite3_column_int(stmt, 2);
    int sellerID = sqlite3_column_int(stmt, 3);
    double amount = sqlite3_column_double(stmt, 4);
    const char* statusStr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
    const char* placedAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
    const char* completedAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
    const char* deadline = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));

   
    Order o;
     o.setOrderID(orderID);
    o.setGigID(gigID);
     o.setBuyerID(buyerID);
    o.setSellerID(sellerID);
 o.setAmount(amount);
    o.setStatus(orderStatusFromString(statusStr ? statusStr : ""));
    o.setPlacedAt(placedAt ? placedAt : "");
    
    o.setCompletedAt(completedAt ? completedAt : "");
    o.setDeadline(deadline ? deadline : "");
    return o;
}



void SQLiteOrderRepository::saveOrder(Order& order) {
    sqlite3* db = DatabaseManager::getInstance().getConnection();

    const string sql =
        "INSERT INTO orders (gigID, buyerID, sellerID, amount, status, "
        "                    placedAt, completedAt, deadline) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?);";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        throwPrepareError(sql);
    }

    const string statusStr = orderStatusToString(order.getStatus());
    const string& completedAt = order.getCompletedAt();

    sqlite3_bind_int(stmt, 1, order.getGigID());
    sqlite3_bind_int(stmt, 2, order.getBuyerID());
     sqlite3_bind_int(stmt, 3, order.getSellerID());
    sqlite3_bind_double(stmt, 4, order.getAmount());
     sqlite3_bind_text(stmt, 5, statusStr.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, order.getPlacedAt().c_str(), -1, SQLITE_TRANSIENT);

    
    if (completedAt.empty()) {
        sqlite3_bind_null(stmt, 7);
    }
    else {
        sqlite3_bind_text(stmt, 7, completedAt.c_str(), -1, SQLITE_TRANSIENT);
    }

    sqlite3_bind_text(stmt, 8, order.getDeadline().c_str(), -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        
        string err = sqlite3_errmsg(db);
        int extended = sqlite3_extended_errcode(db);
        sqlite3_finalize(stmt);

        
        if (rc == SQLITE_CONSTRAINT) 
        {
            if (extended == SQLITE_CONSTRAINT_UNIQUE)
            {
                throw DuplicateEntryException(
                    "Order unique constraint: " + err);
            }
            
            throw ValidationException("Order constraint failed: " + err);
        }
        throw DatabaseException("Failed to insert order: " + err);
    }

    int newID = static_cast<int>(sqlite3_last_insert_rowid(db));
    sqlite3_finalize(stmt);

    order.setOrderID(newID);
}


void SQLiteOrderRepository::updateOrder(const Order& order) {
    sqlite3* db = DatabaseManager::getInstance().getConnection();

    const string sql =
        "UPDATE orders SET gigID=?, buyerID=?, sellerID=?, amount=?, "
        "status=?, placedAt=?, completedAt=?, deadline=? "
        "WHERE orderID=?;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        throwPrepareError(sql);
    }

    const string statusStr = orderStatusToString(order.getStatus());
    const string& completedAt = order.getCompletedAt();

    sqlite3_bind_int(stmt, 1, order.getGigID());
     sqlite3_bind_int(stmt, 2, order.getBuyerID());
    sqlite3_bind_int(stmt, 3, order.getSellerID());
     sqlite3_bind_double(stmt, 4, order.getAmount());
    sqlite3_bind_text(stmt, 5, statusStr.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, order.getPlacedAt().c_str(), -1, SQLITE_TRANSIENT);
    if (completedAt.empty()) {
        sqlite3_bind_null(stmt, 7);
    }
    else {
        sqlite3_bind_text(stmt, 7, completedAt.c_str(), -1, SQLITE_TRANSIENT);
    }
    sqlite3_bind_text(stmt, 8, order.getDeadline().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 9, order.getOrderID());

    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        string err = sqlite3_errmsg(db);
        sqlite3_finalize(stmt);
        throw DatabaseException("Failed to update order: " + err);
    }

    int changed = sqlite3_changes(db);
    sqlite3_finalize(stmt);

    if (changed == 0) {
        throw OrderNotFoundException(
            "No order with orderID=" + to_string(order.getOrderID()));
    }
}



void SQLiteOrderRepository::updateOrderStatus(int orderID,
    OrderStatus newStatus,
    const string& completedAt) {
    sqlite3* db = DatabaseManager::getInstance().getConnection();

    const string sql =
        "UPDATE orders SET status=?, completedAt=? WHERE orderID=?;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        throwPrepareError(sql);
    }

    const string statusStr = orderStatusToString(newStatus);

    sqlite3_bind_text(stmt, 1, statusStr.c_str(), -1, SQLITE_TRANSIENT);
    if (completedAt.empty()) {
        sqlite3_bind_null(stmt, 2);
    }
    else {
        sqlite3_bind_text(stmt, 2, completedAt.c_str(), -1, SQLITE_TRANSIENT);
    }
    sqlite3_bind_int(stmt, 3, orderID);

    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        string err = sqlite3_errmsg(db);
        sqlite3_finalize(stmt);
        throw DatabaseException("Failed to update order status: " + err);
    }

    int changed = sqlite3_changes(db);
    sqlite3_finalize(stmt);

    if (changed == 0) {
        throw OrderNotFoundException(
            "No order with orderID=" + to_string(orderID));
    }
}


bool SQLiteOrderRepository::deleteOrder(int orderID) {
    sqlite3* db = DatabaseManager::getInstance().getConnection();

    const string sql = "DELETE FROM orders WHERE orderID=?;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        throwPrepareError(sql);
    }

    sqlite3_bind_int(stmt, 1, orderID);

    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        string err = sqlite3_errmsg(db);
        sqlite3_finalize(stmt);
        throw DatabaseException("Failed to delete order: " + err);
    }

    int changed = sqlite3_changes(db);
    sqlite3_finalize(stmt);

    return changed > 0;
}



Order SQLiteOrderRepository::findOrderByID(int orderID) const {
    sqlite3* db = DatabaseManager::getInstance().getConnection();

    const string sql =
        "SELECT orderID, gigID, buyerID, sellerID, amount, status, "
        "       placedAt, completedAt, deadline "
        "FROM orders WHERE orderID=?;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        throwPrepareError(sql);
    }

    sqlite3_bind_int(stmt, 1, orderID);

    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        Order o = buildOrderFromRow(stmt);
        sqlite3_finalize(stmt);
        return o;
    }

    sqlite3_finalize(stmt);

    if (rc == SQLITE_DONE) {
        throw OrderNotFoundException(
            "No order with orderID=" + to_string(orderID));
    }

    throw DatabaseException(
        "Failed to query order by ID: " + string(sqlite3_errmsg(db)));
}



DataList<Order> SQLiteOrderRepository::findOrdersByBuyer(int buyerID) const {
    sqlite3* db = DatabaseManager::getInstance().getConnection();

    const string sql =
        "SELECT orderID, gigID, buyerID, sellerID, amount, status, "
        "       placedAt, completedAt, deadline "
        "FROM orders WHERE buyerID=? ORDER BY orderID ASC;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        throwPrepareError(sql);
    }

    sqlite3_bind_int(stmt, 1, buyerID);

    DataList<Order> result;
    int rc;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        result.add(buildOrderFromRow(stmt));
    }

    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        throw DatabaseException(
            "Failed to query orders by buyer: " + string(sqlite3_errmsg(db)));
    }

    return result;
}

DataList<Order> SQLiteOrderRepository::findOrdersBySeller(int sellerID) const {
    sqlite3* db = DatabaseManager::getInstance().getConnection();

    const string sql =
        "SELECT orderID, gigID, buyerID, sellerID, amount, status, "
        "       placedAt, completedAt, deadline "
        "FROM orders WHERE sellerID=? ORDER BY orderID ASC;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        throwPrepareError(sql);
    }

    sqlite3_bind_int(stmt, 1, sellerID);

    DataList<Order> result;
    int rc;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        result.add(buildOrderFromRow(stmt));
    }

    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        throw DatabaseException(
            "Failed to query orders by seller: " + string(sqlite3_errmsg(db)));
    }

    return result;
}


DataList<Order> SQLiteOrderRepository::findOrdersByGig(int gigID) const {
    sqlite3* db = DatabaseManager::getInstance().getConnection();

    const string sql =
        "SELECT orderID, gigID, buyerID, sellerID, amount, status, "
        "       placedAt, completedAt, deadline "
        "FROM orders WHERE gigID=? ORDER BY orderID ASC;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        throwPrepareError(sql);
    }

    sqlite3_bind_int(stmt, 1, gigID);

    DataList<Order> result;
    int rc;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        result.add(buildOrderFromRow(stmt));
    }

    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        throw DatabaseException(
            "Failed to query orders by gig: " + string(sqlite3_errmsg(db)));
    }

    return result;
}


DataList<Order> SQLiteOrderRepository::findAllOrders() const 
{
    sqlite3* db = DatabaseManager::getInstance().getConnection();

    const string sql = "SELECT orderID, gigID, buyerID, sellerID, amount, status, "
        "       placedAt, completedAt, deadline "
        "FROM orders ORDER BY orderID ASC;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        throwPrepareError(sql);
    }

    DataList<Order> result;
    int rc;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        result.add(buildOrderFromRow(stmt));
    }

    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        throw DatabaseException(
            "Failed to query all orders: " + string(sqlite3_errmsg(db)));
    }

    return result;
}