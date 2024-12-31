from flask import Flask, render_template, request, redirect, url_for, flash, session
import subprocess
import os

app = Flask(__name__)
app.secret_key = 'your_secret_key'  # Required for flash messages

CXX_EXEC_PATH = "cpp_backend.exe"  # Path to your compiled C++ executable

# Ensure the C++ executable exists
if not os.path.exists(CXX_EXEC_PATH):
    raise FileNotFoundError(f"Executable not found at {CXX_EXEC_PATH}")

# Function to call the C++ executable with proper error handling
def call_cpp_backend(command, *args):
    try:
        print(f"Executing: {CXX_EXEC_PATH} {command} {' '.join(map(str, args))}")  # Debug line
        result = subprocess.run(
            [CXX_EXEC_PATH, command, *map(str, args)],
            check=True,
            capture_output=True,
            text=True
        )
        print(f"C++ Output: {result.stdout}")  # Debug line
        return result.stdout
    except subprocess.CalledProcessError as e:
        error_msg = f"Error executing C++ backend: {e.stderr.strip() or e.output.strip()}"
        print(error_msg)
        return None

# Function to load items from 'items.txt'
def load_items():
    items = []
    try:
        with open("items.txt", "r") as f:
            for line in f:
                parts = line.strip().split(",")
                if len(parts) == 2:
                    try:
                        item_name = parts[0].strip()
                        item_price = float(parts[1].strip())
                        items.append({"name": item_name, "price": item_price})
                    except (ValueError, IndexError):
                        print(f"Skipping bad formatting: {line}")
    except FileNotFoundError:
        print("Item file does not exist, starting with an empty list!")
    return items

# Cart variable to hold items (you can replace with a database later)
cart = []

# Route to render the home page (this will be the first page opened)
@app.route('/')
def home():
    items = load_items()  # Load items from file or database
    return render_template("home.html", items=items)

# Route to display all items (Index page)
@app.route('/index')
def index():
    if 'admin_logged_in' not in session:  # Check if the user is logged in
        flash('You must log in as an admin to access this page.', 'danger')
        return redirect(url_for('admin_login'))  # Redirect to the login page if not logged in
    
    items = load_items()  # Load items from file or database
    return render_template("index.html", items=items)

# Route to add an item
@app.route('/add_item', methods=['GET', 'POST'])
def add_item():
    if request.method == 'POST':
        new_item = request.form.get("new_item")
        new_price = request.form.get("new_price")
        
        # Check if the new_item and new_price are valid
        if not new_item or not new_price:
            error_message = "Both item name and price are required!"
            print(error_message)  # Debug line
            return render_template("add_item.html", error_message=error_message)

        try:
            new_price = float(new_price)
            # Call C++ backend and check result is not None, only sends the item name to the c++ backend
            result = call_cpp_backend("add", new_item)
            if result is None:
                error_message = "Failed to add item. Please try again."
                print(error_message)  # Debug line
                return render_template("add_item.html", error_message=error_message)
            else:  # If successful, print result and redirect to main page.
                print(result)
                # Instead of just redirecting, load the current items,
                # and manually add the new item into the list
                items = load_items()
                items.append({"name": new_item, "price": new_price})

                # Save the new items to the file.
                with open("items.txt", "w") as f:
                   for item in items:
                       f.write(f"{item['name']},{item['price']}\n")
                
                flash(f'Item "{new_item}" has been added to the bakery.', 'success')  # Flash message
                return redirect(url_for('home'))  # Redirect to home page to show the updated items
        except ValueError:
            error_message = "Invalid price entered. Please enter a valid number."
            print(error_message)  # Debug line
            return render_template("add_item.html", error_message=error_message)
    return render_template("add_item.html")

# Route to update an item
@app.route('/update_item', methods=['GET', 'POST'])
def update_item():
    if request.method == 'POST':
        old_item = request.form.get("old_item")
        new_item = request.form.get("new_item", old_item)  # Default to old_item if new_item is empty
        new_price = request.form.get("new_price")
        
        if not new_item or not new_price:
            error_message = "Item name and price are required!"
            print(error_message)  # Debug line
            return render_template("update_item.html", error_message=error_message)
        
        try:
            # Convert new_price to float if provided
            if new_price:
                new_price = float(new_price)

            # Call C++ backend to update item name
            result_name = call_cpp_backend("update", old_item, new_item)
            if result_name is None:
                error_message = "Failed to update item name. Please try again."
                print(error_message)  # Debug line
                return render_template("update_item.html", error_message=error_message)

            # Load current items and update the item details
            items = load_items()
            for item in items:
                if item["name"] == old_item:
                    item["name"] = new_item
                    if new_price is not None:
                        item["price"] = new_price

            # Save the updated items to the file.
            with open("items.txt", "w") as f:
                for item in items:
                    f.write(f"{item['name']},{item['price']}\n")

            flash(f'Item "{old_item}" has been updated to "{new_item}".', 'success')  # Flash message
            return redirect(url_for('home'))  # Redirect to home page to show the updated items
        
        except ValueError:
            error_message = "Invalid price entered. Please enter a valid number."
            print(error_message)  # Debug line
            return render_template("update_item.html", error_message=error_message)
    
    return render_template("update_item.html")

# Route to remove an item
@app.route('/remove_item', methods=['GET', 'POST'])
def remove_item():
    if request.method == 'POST':
        item_name = request.form.get("item_name")
        
        if not item_name:
            error_message = "Item name is required!"
            print(error_message)  # Debug line
            return render_template("remove_item.html", error_message=error_message)

        # Call C++ backend to remove the item
        result = call_cpp_backend("remove", item_name)
        if result is None:
            error_message = "Failed to remove item. Please try again."
            print(error_message)  # Debug line
            return render_template("remove_item.html", error_message=error_message)
        
        # Load current items and remove the item
        items = load_items()
        items = [item for item in items if item["name"] != item_name]
        
        # Save the updated items to the file.
        with open("items.txt", "w") as f:
            for item in items:
                f.write(f"{item['name']},{item['price']}\n")

        flash(f'Item "{item_name}" has been removed from the bakery.', 'success')  # Flash message
        return redirect(url_for('home'))  # Redirect to home page to show the updated items
    
    return render_template("remove_item.html")

# Route to handle buying an item
@app.route('/buy_item', methods=['POST'])
def buy_item():
    item_name = request.form.get('item_name')
    item_price = request.form.get('item_price')
    
    if not item_name or not item_price:
        error_message = "Item name and price are required!"
        print(error_message)  # Debug line
        return redirect(url_for('home'))  # Redirect to home page with error
    
    # Add the item to the cart
    cart.append({"name": item_name, "price": float(item_price)})

    flash(f'Item "{item_name}" added to the cart.', 'success')  # Flash message
    return redirect(url_for('home'))  # Redirect to home page with success message

# Route to view the cart
@app.route('/view_cart')
def view_cart():
    return render_template("view_cart.html", cart=cart)

@app.route('/place_order')
def place_order():
    total_amount = sum(item['price'] for item in cart)
    return render_template('place_order.html', cart=cart, total_amount=total_amount)

@app.route('/submit_order', methods=['POST'])
def submit_order():
    if not cart:
        flash('Cannot place order with empty cart', 'error')
        return redirect(url_for('view_cart'))

    customer_name = request.form.get('customer_name')
    email = request.form.get('email')

    if not customer_name or not email:
        flash('Please provide both name and email', 'error')
        return redirect(url_for('place_order'))

    # Convert cart items to command-line arguments format
    items_args = []
    for item in cart:
        items_args.extend([item['name'], str(item['price'])])

    # Call C++ backend to place order
    result = call_cpp_backend("place_order", customer_name, email, str(len(cart)), *items_args)
    
    if result is None:
        flash('Error placing order. Please try again.', 'error')
        return redirect(url_for('place_order'))

    # Clear the cart after successful order placement
    cart.clear()
    
    flash('Order placed successfully!', 'success')
    return redirect(url_for('home'))

@app.route('/view_orders')
def view_orders():
    if 'admin_logged_in' not in session:
        flash('You must log in as an admin to access this page.', 'danger')
        return redirect(url_for('admin_login'))

    result = call_cpp_backend("view_orders")
    if result is None:
        flash('Error retrieving orders.', 'error')
        orders = []
    else:
        # Split the orders by double newline to separate individual orders
        orders = [order.strip() for order in result.strip().split('\n\n') if order.strip()]
    
    return render_template('view_orders.html', orders=orders)

@app.route('/process_order', methods=['POST'])
def process_order():
    if 'admin_logged_in' not in session:
        flash('You must log in as an admin to access this page.', 'danger')
        return redirect(url_for('admin_login'))

    result = call_cpp_backend("process_order")
    if result is None:
        flash('Error processing order.', 'error')
    else:
        flash('Order processed successfully!', 'success')
    
    return redirect(url_for('view_orders'))

@app.route('/admin_login', methods=['GET', 'POST'])
def admin_login():
    if request.method == 'POST':
        username = request.form.get('username')
        password = request.form.get('password')
        
        # Check if username and password are correct
        if username == 'admin' and password == 'admin123':
            session['admin_logged_in'] = True  # Set session to remember the admin is logged in
            flash('Login successful!', 'success')
            return redirect(url_for('index'))  # Redirect to index page after successful login
        else:
            flash('Invalid username or password. Please try again.', 'danger')  # Flash error message
            return render_template('admin_login.html')  # Re-render the login page
    
    return render_template('admin_login.html')


if __name__ == "__main__":
    app.run(debug=True)
