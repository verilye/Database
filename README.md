## Database Project

### Goal

In its final form, this will be an implementation of a NOSQL database with all the trimmings,
fully presentable to a professional degree and able to be used practically and distributed if
so inclined.

### Design decisions

At this point my mental map of how the database will function is as follows. Any number of clients
will be able to access the server and send CRUD instructions to the data inside. There will be a
priority system to instructions sent to the database, and those who get in first get to execute their
instructions first. Once a change is made, everyone will have access to the altered dataset and rejected
potentially if assumptions on the data are no longer true. The data will be stored inside a combination of 
data structures (Hash table and AVL tree?) and formatted data will be inserted into it and come out when prompted. 

### Reading Material

Basic outline of what to do is found in implement Redis from scratch, a book about how to 
implement a NOSQL database. No code should be copied from there and it should be used only 
as a general pointer on where to focus your attention next.

### TODO
- revisit current code and correct for cleanliness
- remove chatting functionality and repurpose client and server to just socket connect
- update connecting protocol so that its a little more sophisticated
    - what is being sent and recieved will need to be revamped totally to fit the new protocol
- implement the event loop and nonblocking IO
- Basic server functions - get, set, del
- Implement a hash table
- serialise data
- implement an AVL tree
- get a sorted set from an AVL tree
- implement TTL (Time To Live) timeouts 
- Implement a thread pool and asynchronous tasks
