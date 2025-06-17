.. _context_broker_interface:

########################
Context Broker Interface
########################

The Context Broker interface provides three key callbacks that facilitate interaction with the DDS network.
Two callbacks handle incoming data from the DDS network, while the third manages logging information related
to DDS network activities.

These callbacks are defined in the header file ``CBCallbacks.h``. You must set up the implementation of these callbacks
during the initialization process of the ``DDS Enabler`` by using the ``init_dds_enabler`` function.

.. code-block:: c++

    int init_dds_enabler(
            const char* ddsEnablerConfigFile,
            participants::DdsDataNotification data_callback,
            participants::DdsTypeNotification type_callback,
            participants::DdsLogFunc log_callback);

Data callback
=============

The data callback is used to add new data to a specific attribute (DDS Topic) of an NGSI-LD entity.

It is triggered by the ``CBHandler`` via the ``CBWriter`` class.

.. code-block:: c++

    typedef void (*DdsNotification)(
            const char* typeName,
            const char* topicName,
            const char* json,
            double publishTime);

Type discovery callback
========================

The type discovery callback registers newly discovered DDS types in the network.
It stores the topic name, data type, and type description (IDL) in a metadata table,
which is used by eProsima to create publishers or subscribers for these topics.

This callback is invoked by the ``CBHandler`` through the ``CBWriter`` class.

.. code-block:: c++

    typedef void (*DdsTypeNotification)(
            const char* typeName,
            const char* topicName,
            const char* serializedType);

.. TODO: update with topic notification, topic request and type request callbacks

Log callback
============
The Log Callback is responsible for relaying all relevant logging information from the Fast DDS ecosystem to the
Context Broker.

This callback is triggered by the ``DDSEnablerLogConsumer``.

.. code-block:: c++

    typedef void (*DdsNotification)(
            const char* typeName,
            const char* topicName,
            const char* json,
            double publishTime);
