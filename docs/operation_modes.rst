.. _operation_modes:

###############
Operation Modes
###############

Legacy Mode
===========

Mapping DDS data to NGSI-LD format
----------------------------------

There are four different cases depending on how the type's attributes are marked.

No Keys
^^^^^^^

The DDS type does not have any attributes marked as key.
All samples go to the same NGSI-LD entity, which may or not be specified in the Context Broker configuration file.

*   DDS Data Type

.. code-block:: idl

    struct TemperatureReadings
    {
        int sensor_id;
        int building;
        int temperature;
    };

*   NGSI-LD Entity

.. code-block:: json

    {
        "id": "<fastdds-participant-guid>",
        "type": "fastdds",
        "temperature_readings": { // topic name
            "sensor_id": "1",
            "building": "1",
            "temperature": 10
        }
    }


Single or multiple keys without configuration
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The keys will be translated into a unique key hash identifier of the key.
This key has will be provided by the DDS Pipe.

*   DDS Data Type

.. code-block:: idl

    struct TemperatureReadings
    {
        @key int sensor_id;
        @key int building;
        int temperature;
    };

*   NGSI-LD Entity

.. code-block:: json

    {
        "id": "<fastdds-participant-guid>",
        "type": "fastdds",
        "temperature_readings": {
            "key_hash_1": {
                "sensor_id": "1",
                "building": "1",
                "temperature": 10
            },
            "key_hash_2": {
                "sensor_id": "1",
                "building": "1",
                "temperature": 10
            }
        }
    }

Single key with configuration file
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

There is only one key member in the DDS data type.

The user will be able to assign a key member of one specific topic to an specific NGSI-LD entity.
Something similar to: "the topic X with key member Y and value Z goes to entity A".

*   DDS Data Type

.. code-block:: idl

    struct TemperatureReadings
    {
        int sensor_id;
        @key int building;
        int temperature;
    };

*   NGSI-LD Entity

.. code-block:: json

    {
        "id": "building_1",
        "type": "fastdds",
        "temperature_readings": {
            "key_hash_1": {
                "sensor_id": "1",
                "building": "1", // The key identifies the entity
                "temperature": 10
            }
        }
    },
    {
        "id": "building_2",
        "type": "fastdds",
        "temperature_readings": {
            "key_hash_2": {
                "sensor_id": "1",
                "building": "2",
                "temperature": 10
            }
        }
    }

Single key with configuration file
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

There are multiple key members in the DDS data type.

The user will be able to assign one or multiple key member of one specific topic to an specific NGSI-LD entity.
Something similar to: "the topic X with key member Y_1 and value Z_1, and key member X_2 and value Z_2, and ...,
goes to entity A".

*   DDS Data Type

.. code-block:: idl
    struct TemperatureReadings
    {
        @key int sensor_id;
        @key int building;
        int temperature;
    };

*   NGSI-LD Entity

.. code-block:: json

    {
        "id": "building_1",
        "type": "fastdds",
        "temperature_readings": {
            "key_hash_1": { // Key has of "sensor_id" and "building" values
                "sensor_id": "1",
                "building": "1", // Only this key identifies the entity
                "temperature": 10
            },
            "key_hash_11": { // Key has of "sensor_id" and "building" values
                "sensor_id": "2",
                "building": "1", // Only this key identifies the entity
                "temperature": 10
            },
            "key_hash_111": { // Key has of "sensor_id" and "building" values
                "sensor_id": "3",
                "building": "1", // Only this key identifies the entity
                "temperature": 10
            }
        }
    },
    {
        "id": "building_2",
        "type": "fastdds",
        "temperature_readings": {
            "key_hash_2": {
                "sensor_id": "1",
                "building": "2",
                "temperature": 10
            },
            "key_hash_22": {
                "sensor_id": "2",
                "building": "2",
                "temperature": 10
            }
        }
    }

DDS Binding
===========

Configuration files are used to stablish relationships between Topics and attributes.
