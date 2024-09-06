# PCom Homework #4

The implementation of this homework is started from lab 9

Changes made : client.c , request.c

Request.c:
We have two helper functions called:
1) add_cookies_to_request: in which this function adds a cookie header to an HTTP request message. The header includes the first cookie from a list of cookies.
2) add_jwt_to_request: in which this function adds an authorization header with a JWT to an HTTP request message.

and three main functions called:
1) compute_get_request: This function prepares a GET request to be sent to the server.
2) compute_post_request: This function prepares a POST request to be sent to the server.
3) compute_delete_request: This function prepares a DELETE request to be sent to the server


Client.c:

Main: The entry point of our application. It parses command-line arguments, establishes a connection with the server, and initiates the request-response cycle.

1) send_request_and_receive_response: sends a request to server and receives a response.
2) register_client: here the client registers for the first time.
3) login_client: these command allows the user to open his already registered account.
4) enter_library: here the client enter the library in which he can acces diferent books, add or even delete them.
5) get_books: with these command the user can access all the books the he saved in the library.
6) get_book: and he also can choose which book to view info from only by using the book id.
7) add_book: while being in the library the user can decide to add different books.
8) delete_book: also he can remove them from library
9) logout: after logging out the user cannot access the library commands anymore.


Parson.c:

1) The "json_value_init_object" function is part of the Parson JSON library in C. It initializes a new JSON object value.

2) This function returns a JSON_Value structure that represents an empty JSON object {}. You can then get the JSON_Object from this value using "json_value_get_object" and start setting fields in this object using functions like "json_object_set_string".

3) json_serialize_to_string :this function takes a JSON_Value structure as input and converts it into a string representation of the JSON data.