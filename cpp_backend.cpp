/*
MEMBERS:
Manahil Ahmad -231557
Sumaiyyah Masood -231596
Rameen Aizad -231602
*/

#include <iostream>
#include <string>
#include <cstdlib>
#include <fstream>
#include <sstream>

using namespace std;

const int MAX_ITEMS = 100;
const int MAX_ORDERS = 100;
const string ORDERS_FILE = "orders.txt";

struct OrderItem
{
    string name;
    double price;
};

struct Order
{
    string customerName;
    string email;
    OrderItem items[MAX_ITEMS];
    int itemCount;
    double totalAmount;
    Order *next;

    Order() : next(nullptr), itemCount(0), totalAmount(0) {}
};

class OrderQueue
{
private:
    Order *front;
    Order *rear;

public:
    OrderQueue() : front(nullptr), rear(nullptr)
    {
        loadOrders(); // Load orders from file when queue is created
    }

    ~OrderQueue()
    {
        saveOrders(); // Save orders before destruction
        while (front != nullptr)
        {
            Order *temp = front;
            front = front->next;
            delete temp;
        }
    }

    void loadOrders()
    {
        ifstream file(ORDERS_FILE);
        if (!file.is_open())
            return;

        string line;
        Order *currentOrder = nullptr;

        while (getline(file, line))
        {
            if (line.empty())
            {
                currentOrder = nullptr;
                continue;
            }

            if (line.find("Customer: ") == 0)
            {
                currentOrder = new Order();
                currentOrder->customerName = line.substr(10);
                if (front == nullptr)
                {
                    front = rear = currentOrder;
                }
                else
                {
                    rear->next = currentOrder;
                    rear = currentOrder;
                }
            }
            else if (currentOrder && line.find("Email: ") == 0)
            {
                currentOrder->email = line.substr(7);
            }
            else if (currentOrder && line.find("- ") == 0)
            {
                // Parse item line: "- ItemName ($Price)"
                string itemInfo = line.substr(2);
                size_t priceStart = itemInfo.find(" ($");
                if (priceStart != string::npos)
                {
                    string itemName = itemInfo.substr(0, priceStart);
                    double price = stod(itemInfo.substr(priceStart + 3, itemInfo.length() - priceStart - 4));
                    currentOrder->items[currentOrder->itemCount] = {itemName, price};
                    currentOrder->totalAmount += price;
                    currentOrder->itemCount++;
                }
            }
        }
        file.close();
    }

    void saveOrders()
    {
        ofstream file(ORDERS_FILE);
        if (!file.is_open())
            return;

        Order *current = front;
        while (current != nullptr)
        {
            file << "Customer: " << current->customerName << endl;
            file << "Email: " << current->email << endl;
            for (int i = 0; i < current->itemCount; i++)
            {
                file << "- " << current->items[i].name << " ($" << current->items[i].price << ")" << endl;
            }
            file << "Total Amount: $" << current->totalAmount << endl
                 << endl;
            current = current->next;
        }
        file.close();
    }

    void enqueue(const string &customerName, const string &email, const OrderItem items[], int itemCount)
    {
        if (itemCount > MAX_ITEMS)
        {
            throw runtime_error("Too many items in order");
        }

        Order *newOrder = new Order();
        newOrder->customerName = customerName;
        newOrder->email = email;
        newOrder->itemCount = itemCount;

        double total = 0;
        for (int i = 0; i < itemCount; i++)
        {
            newOrder->items[i] = items[i];
            total += items[i].price;
        }
        newOrder->totalAmount = total;

        if (rear == nullptr)
        {
            front = rear = newOrder;
        }
        else
        {
            rear->next = newOrder;
            rear = newOrder;
        }

        saveOrders(); // Save orders after adding new one
    }

    bool dequeue()
    {
        if (front == nullptr)
            return false;

        Order *temp = front;
        front = front->next;
        if (front == nullptr)
            rear = nullptr;

        delete temp;
        saveOrders(); // Save orders after removing one
        return true;
    }

    string getOrdersList() const
    {
        if (front == nullptr)
            return "No orders in queue.\n";

        string ordersList;
        Order *current = front;
        int orderNum = 1;

        while (current != nullptr)
        {
            ordersList += "Order #" + to_string(orderNum) + "\n";
            ordersList += "Customer: " + current->customerName + "\n";
            ordersList += "Email: " + current->email + "\n";
            ordersList += "Items:\n";

            for (int i = 0; i < current->itemCount; i++)
            {
                ordersList += "- " + current->items[i].name +
                              " ($" + to_string(current->items[i].price) + ")\n";
            }

            ordersList += "Total Amount: $" + to_string(current->totalAmount) + "\n\n";

            current = current->next;
            orderNum++;
        }
        return ordersList;
    }
};

// Define a structure for the Node of the linked list
struct Node
{
    string item;
    Node *next;
    Node(string _item) : item(_item), next(nullptr) {}
};

// Define a class for LinkedList to manage items
class LinkedList
{
public:
    Node *head;

    LinkedList() : head(nullptr) {}

    // Function to add an item to the list
    void addItem(const string &item)
    {
        Node *newNode = new Node(item);
        if (!head)
        {
            head = newNode;
        }
        else
        {
            Node *temp = head;
            while (temp->next)
            {
                temp = temp->next;
            }
            temp->next = newNode;
        }
    }

    // Function to update an item in the list
    bool updateItem(const string &old_item, const string &new_item)
    {
        Node *temp = head;
        while (temp)
        {
            if (temp->item == old_item)
            {
                temp->item = new_item;
                return true;
            }
            temp = temp->next;
        }
        return false;
    }

    // Function to remove an item from the list
    bool removeItem(const string &item)
    {
        if (!head)
            return false;

        if (head->item == item)
        {
            Node *temp = head;
            head = head->next;
            delete temp;
            return true;
        }

        Node *temp = head;
        while (temp->next)
        {
            if (temp->next->item == item)
            {
                Node *toDelete = temp->next;
                temp->next = temp->next->next;
                delete toDelete;
                return true;
            }
            temp = temp->next;
        }
        return false;
    }

    // Function to display all items in the list
    void displayItems() const
    {
        if (!head)
        {
            cout << "The list is empty." << endl;
            return;
        }

        Node *temp = head;
        while (temp)
        {
            cout << temp->item << endl;
            temp = temp->next;
        }
    }

    // Function to get the list of items as a string
    string getItemsList() const
    {
        if (!head)
        {
            return "The list is empty.\n";
        }

        string itemsList = "";
        Node *temp = head;
        while (temp)
        {
            itemsList += temp->item + "\n";
            temp = temp->next;
        }
        return itemsList;
    }

    // Destructor to delete the linked list and free memory
    ~LinkedList()
    {
        Node *temp = head;
        while (temp)
        {
            Node *toDelete = temp;
            temp = temp->next;
            delete toDelete;
        }
    }
};

LinkedList itemsList;
OrderQueue orderQueue;

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        cerr << "Error: Insufficient arguments provided!" << endl;
        return 1;
    }

    string command = argv[1];

    try
    {
        // Previous commands remain the same...
        if (command == "add" && argc == 3) // Add item
        {
            string item = argv[2];
            itemsList.addItem(item);

            // Output the updated list of item
            cout << "Item added successfully!\n";
            cout << "Updated Items List:\n"
                 << itemsList.getItemsList();
        }
        else if (command == "update" && argc == 4) // Update item
        {
            string old_item = argv[2];
            string new_item = argv[3];
            if (itemsList.updateItem(old_item, new_item))
            {
                cout << "Item updated successfully!\n";
                cout << "Updated Items List:\n"
                     << itemsList.getItemsList();
            }
            else
            {
                cerr << "Error: Item '" << old_item << "' not found!" << endl;
            }
        }
        else if (command == "remove" && argc == 3) // Remove item
        {
            string item = argv[2];
            if (itemsList.removeItem(item))
            {
                cout << "Item removed successfully!\n";
                cout << "Updated Items List:\n"
                     << itemsList.getItemsList();
            }
            else
            {
                cerr << "Error: Item '" << item << "' not found!" << endl;
            }
        }
        else if (command == "display") // Display items
        {
            cout << "Items in the list:\n";
            itemsList.displayItems();
        }
        if (command == "place_order" && argc >= 5)
        {
            string customerName = argv[2];
            string email = argv[3];
            int itemCount = stoi(argv[4]);

            if (argc < 5 + itemCount * 2)
            {
                cerr << "Error: Not enough item details provided!" << endl;
                return 1;
            }

            OrderItem items[MAX_ITEMS];
            for (int i = 0; i < itemCount; i++)
            {
                items[i].name = argv[5 + i * 2];
                items[i].price = stod(argv[6 + i * 2]);
            }

            orderQueue.enqueue(customerName, email, items, itemCount);
            cout << "Order placed successfully!\n";
            cout << "Current Orders:\n"
                 << orderQueue.getOrdersList();
        }
        else if (command == "process_order")
        {
            if (orderQueue.dequeue())
            {
                cout << "Order processed successfully!\n";
                cout << "Remaining Orders:\n"
                     << orderQueue.getOrdersList();
            }
            else
            {
                cout << "No orders to process.\n";
            }
        }
        else if (command == "view_orders")
        {
            cout << "Current Orders:\n"
                 << orderQueue.getOrdersList();
        }
        else
        {
            cerr << "Error: Invalid command!" << endl;
        }
    }
    catch (const exception &e)
    {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }

    return 0;
}