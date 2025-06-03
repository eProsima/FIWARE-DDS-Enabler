.. include:: ../exports/alias.include
.. include:: ../exports/roles.include

.. _project_overview:

################
Project Overview
################

Enabling technologies
=====================

This section describes the different technologies that support the development of the AML-IP.
All of them focus on the communication between nodes, the protocols used to support such communication and the tools
used to handle the different types of data to be transmitted.

Data Distribution Service (DDS)
-------------------------------

Given the distributed nature of the AML ecosystem, the use of DDS is proposed to enable real-time communication between
AML nodes.

As explained in the eProsima Fast DDS documentation:

.. note::

    The Data Distribution Service (DDS) is a data-centric communication protocol used for distributed
    software application communications.
    It describes the communications Application Programming Interfaces (APIs) and Communication Semantics
    that enable communication between data providers and data consumers.

    Since it is a Data-Centric Publish Subscribe (DCPS) model, three key application entities are
    defined in its implementation:

    * publication entities, which define the information-generating objects and their properties.
    * subscription entities, which define the information-consuming objects and their properties.
    * configuration entities, that define the types of information that are transmitted as topics,
      and create the publisher and subscriber with its Quality of Service (Quality of Service (QoS)) properties,
      ensuring the correct performance of the above entities.

    DDS uses QoS to define the behavioral characteristics of DDS Entities. QoS are comprised
    of individual QoS policies (objects of type deriving from QoSPolicy).

The different entities involved in the DDS protocol are also defined in the eProsima Fast DDS documentation.

.. note::

    In the DCPS model, four basic elements are defined for the development of a system of communicating applications.

    * Publisher: It is the DCPS entity in charge of the creation and configuration of the DataWriters it implements.
      The DataWriter is the entity in charge of the actual publication of the messages.
      Each one will have an assigned Topic under which the messages are published.
    * Subscriber: It is the DCPS Entity in charge of receiving the data published under the topics to which it
      subscribes.
      It serves one or more DataReader objects, which are responsible for communicating the availability of new data to
      the application.
    * Topic: It is the entity that binds publications and subscriptions. It is unique within a DDS domain.
      Through the TopicDescription, it allows the uniformity of data types of publications and subscriptions.
      See Topic for further details.
    * Domain: This is the concept used to link all publishers and subscribers, belonging to one or more applications,
      which exchange data under different topics.
      These individual applications that participate in a domain are called DomainParticipant.
      The DDS Domain is identified by a domain ID.
      The DomainParticipant defines the domain ID to specify the DDS domain to which it belongs.
      Two DomainParticipants with different IDs are not aware of each other’s presence in the network.
      Hence, several communication channels can be created.
      This is applied in scenarios where several DDS applications are involved, with their respective
      DomainParticipants communicating with each other, but these applications must not interfere.
      The DomainParticipant acts as a container for other DCPS Entities, acts as a factory for Publisher, Subscriber and
      Topic Entities, and provides administrative services in the domain.
      See Domain for further details.

The dynamic addition of new entities to the network is also covered by the the DDS protocol, allowing the dynamic
discovery of new instantiated DDS entities.
The Discovery Protocol is the mechanism that DDS provides for the discovery of new entities.

Thus, DDS fulfills various communication requirements of the AML outlined in Section 2.4. However,
the WAN communication, and specially the WAN dynamic discovery of newly instantiated DDS entities is a great
challenge to DDS, as this protocol relays on multicast communication. The growing type of nodes and data formats
makes a challenge in order to achieve fast DDS deployments over WAN.


eProsima Fast DDS
-----------------

As described in the Fast DDS documentation:

.. note::

    eProsima Fast DDS is a C++ implementation of the DDS (Data Distribution Service) Specification,
    a protocol defined by the Object Management Group (OMG).
    The eProsima Fast DDS library provides both an Application Programming Interface (API) and a communication protocol
    that deploy a Data-Centric Publisher-Subscriber (DCPS) model, with the purpose of establishing efficient and
    reliable information distribution among Real-Time Systems.
    eProsima Fast DDS is predictable, scalable, flexible, and efficient in resource handling.
    For meeting these requirements, it makes use of typed interfaces and hinges on a many-to-many distributed network
    paradigm that neatly allows separation of the publisher and subscriber sides of the communication.


Among the many features of Fast DDS, there are two key functionalities for the development of the AML-IP:
a) the TCP transport,
b) the Discovery Server discovery mechanism.

Among the multiple transport protocols and communication mechanisms available in Fast DDS, the TCP transport protocol
is implemented for data transmission between DDS entities in WAN networks.
However, the configuration of DDS entities to use this protocol is not straightforward at this point.
The purpose of the AML-IP is to implement a communications framework that uses the Transmission Control Protocol (TCP)
communications protocol implemented in Fast DDS in a transparent manner for the user.

Moreover, according to the eProsima Fast DDS documentation:

.. note::

    Fast DDS, as a Data Distribution Service (DDS) implementation, provides discovery mechanisms that allow for
    automatically finding and matching DataWriters and DataReaders across DomainParticipants so they can start
    sharing data.

The AML-IP framework will exploit the Discovery Server discovery mechanism described in the eProsima Fast DDS
documentation as follows:

.. note::

    This mechanism [Discovery Server] is based on a client-server discovery paradigm, i.e. the metatraffic
    (message exchange among DomainParticipants to identify each other) is managed by one or several server
    DomainParticipants [...], as opposed to simple discovery [...], where metatraffic is exchanged using a
    message broadcast mechanism like an IP multicast protocol.

The Fast DDS Discovery Server is thought to reduce the traffic network and to assist in DDS discovery in cases where
multicast is not an option in a dynamically scalable DDS networks.
Moreover, it centralizes the discovery information for the Fast DDS DomainParticipants connected to it.
There are some key aspects that need to be covered in order to understand the Discovery Server discovery mechanism.

The following can be gleaned from the Fast DDS online documentation:

* Discovery Server DomainParticipants may be clients or servers.
  The only difference between them is on how they handle discovery traffic.
  The user traffic [...] is role-independent.
  [...]
* All server and client discovery information will be shared with linked clients.
  [...] a server may act as a client for other servers.
* A SERVER is a participant to which the clients (and maybe other servers) send their discovery information.
  The role of the server is to re-distribute the clients (and servers) discovery information to their known clients and
  servers.
  A server may connect to other servers to receive information about their clients.
  Known servers will receive all the information known by the server.
  Known clients will only receive the information they need to establish communication,
  i.e. the information about the DomainParticipants, DataWriters, and DataReaders to which they match.
  This means that the server runs a “matching” algorithm to sort out which information is required by which client.
  [...]
* A CLIENT is a participant that connects to one or more servers from which it receives only the discovery information
  they require to establish communication with matching endpoints.


eProsima DDS Pipe
-----------------

eProsima DDS Pipe is a foundational component utilized across various eProsima products to facilitate seamless
communication between different applications within a DDS (Data Distribution Service) environment.
It efficiently handles the creation and discovery of DDS participants, ensuring smooth and reliable communication
with the DDS ecosystem.
The module ensures that payloads are efficiently transmitted, minimizing latency and maximizing throughput.

DDS Enabler
===========

DDS Enabler serves as a single point of entry for DDS communication, managing all necessary DDS participants and
efficiently transmitting the DDS data published into a DDS environment to the Context Broker.

DDS Enabler supports extensive configuration options through a YAML file.
This allows users to fine-tune various DDS settings to meet their specific requirements.
Including easily managing topic visibility by blacklisting or whitelisting different topics,
providing granular control over data distribution.

DDS Enabler can be configured, so that DDS topics, data types and entities are automatically discovered without the
need to specify the types of data recorded.
This is because it exploits the DynamicTypes functionality of eProsima Fast DDS, the C++ implementation of the
DDS (Data Distribution Service) Specification defined by the Object Management Group (OMG).

CBHandler
---------

The class that manages the interaction between DDS Pipe \c (SchemaParticipant) and CB.
Payloads are efficiently passed from DDS Pipe to CB without copying data (only references).
