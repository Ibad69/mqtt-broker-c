creating a mqtt protocol just for fun in c.....not sure if that is best idea but I don't know let's see..


-
What we are going to be requiring to build a MQTT protocol
-


**we need a place or a protocol through which clients are going to connect to us in other words maybe a tcp connection? not sure will need to look into some networking conecpts** 
**the tcp connection is going to enable for the other clients to connect to us**

-- accept a tcp handshake first
-- manage packets
-- manage those connections?
-- manage all the connected people
-- create a place for them to create rooms.
-- rooms will have the ability to subscribe 
-- subscribed connection will receive the data that is being propogated in the room? not sure if that sounds fully correct but it is what it is...
-- manage deconnections
-- throw data in a timely manner since this is going to be somewhat of my own personal use case for a mqtt I want it to throw some data (I wanna test a mqtt backend that I have created with the populated data)
-- transfer of packets definitely how is this gonna work? I have no fk*** idea xD
-- is this going to be a broker?
-- backpressure management and memory management also


--------------------------------------------------


-- so some networking concepts we'll have to understand like the ipv4 and ipv6 mechanism, it basically is needed to build up a system of your own particular requirements
    there is also a byte order scenario where if we are accepting a particular packet we'll have to "Basically, you’ll want to convert the numbers to Network Byte Order before they go out on the wire, and convert them to Host Byte Order as they come in off the wire."

-- struct addrinfo --> we will be loading this struct up a bit and call getaddrifno, it will basically give us a pointer to a linked list of these structures

-- initialize a addrinfo


----------------------------------------------
how packets in the mqtt library are generally working 

so there is a uint_8t buf, it is being passed in the receive call with void so that it can be manipulated accordingly, the values change depending on the buffer that is being processed

--- **packet shifting** ---
Right shift: When the info you want is in the most significant bits of a byte (like packet type), so you bring them down to the low end.

Left shift: When building a multi-byte number from least significant to most significant parts, you shift new chunks up to their correct place.
