.. _context_broker_interface:

########################
Context Broker Interface
########################

The Context Broker interface includes three key callbacks that facilitate interaction with the DDS network.
Two of these callbacks are designed for adding new data coming from the DDS network into the system,
while the third handles the logging information related to DDS network activities.

Data callback
=============
This callback is used to add new data to a specific attribute (DDS Topic) of an NGSI-LD entity.

It is invoked by the ``CBHandler`` through the ``CBWriter`` class.

Topic discovery callback
========================
This callback is responsible for registering newly discovered DDS topics in the network.
The function adds the topic name, data type, and type description (TypeObject) to a metadata table.
This data is used when eProsima needs to create publishers or subscribers for these topics to publish data.

It is invoked by the ``CBHandler`` through the ``CBWriter`` class.

Log callback
============
The Log Callback is responsible for relaying all relevant logging information from the Fast DDS ecosystem to the
Context Broker.

This callback is triggered by the ``DDSEnablerLogConsumer``.
