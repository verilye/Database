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

#### client.cpp and server.cpp

These are the data brokers, actual database logic will be seperated into seperate files 

### Reading Material

Basic outline of what to do is found in implement Redis from scratch, a book about how to 
implement a NOSQL database. No code should be copied from there and it should be used only 
as a general pointer on where to focus your attention next.

## Notes on Database Design Theory

https://yunpengn.github.io/blog/2019/01/20/understanding-database-storage/

There is a memory hierarchy of devices, registers at the top, then On-Chip L1 cache, off chip l1 cache, Main memory (DRAM), local disks then remote secondary storage, 
listed in order of speed and ease of access. Higher up the hierarchy means faster transfer but lower volume. Relational Databases use harddrives as a persistent layer
and main memory to store things being currently processed. 

The cost of 'page' I/O is the primary consideration of databases. Typically a page is around 4-8KB.
Use a term record id which could identify a record's disk address. 

Databases store their data in varied ways, most often in the file system of the host machine split up into files of varying sizes (in edge cases, no filesystem used).
Potentially use a database friendly partition on a drive/usb for reading/writing (USB with XFS for example)

(Pulled from Google Cloud, what is nosql)
NOSQL databases come in a number of flavours:
- Document (data is stored in documents using JSON or something similar, create and update applications without referencing a schema)
- Key-value (simple key value store, unique key is paired with a value)
- Column (store and read data in rows. Column names and formatting can vary from row to row in a single table)
- Graph (data is organised in nodes, focusing on relationships between nodes. Connections or edges are stored)
- In-Memory (in memory provides no latency for real time applications, Redis is an example)

For my purposes it seems like it would be a good idea to implement a combination between columns and rows + key value. 
These concepts can be applied to pretty much any database, so they would be the most widely applicable. COOL!
Refer to Apache Cassandra and MongoDb for cool examples of NOSQL databases. Also think about doing some big data operations
on my own database 



### TODO
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
