.. _configuration:

#############
Configuration
#############

DDS Enabler Configuration
========================

The DDSEnabler is configured by a *.yaml* configuration file.
This *.yaml* file contains all the information regarding the DDS interface configuration, and DDSEnabler specifications.
Thus, this file has three major configuration groups:

* ``dds``: configuration related to DDS communication.
* ``ddsenabler``: configuration of data writing in the database.
* ``specs``: configuration of the internal operation of the DDSEnabler.

.. _ddsenabler_dds_configuration:

DDS Configuration
-----------------

Configuration related to DDS communication.

.. _ddsenabler_domain_id:

DDS Domain
^^^^^^^^^^

Tag ``domain`` configures the :term:`Domain Id`.

.. code-block:: yaml

    domain: 101

.. _ddsenabler_topic_filtering:

Topic Filtering
^^^^^^^^^^^^^^^

The DDSEnabler automatically detects the topics that are being used in a DDS Network.
The DDSEnabler then creates internal DDS :term:`Readers<DataReader>` to record the data published on each topic.
The DDSEnabler allows filtering DDS :term:`Topics<Topic>` to allow users to configure the DDS :term:`Topics<Topic>` that must be recorded.
These data filtering rules can be configured under the ``allowlist`` and ``blocklist`` tags.
If the ``allowlist`` and ``blocklist`` are not configured, the DDSEnabler will recorded the data published on every topic it discovers.
If both the ``allowlist`` and ``blocklist`` are configured and a topic appears in both of them, the ``blocklist`` has priority and the topic will be blocked.

Topics are determined by the tags ``name`` (required) and ``type``, both of which accept wildcard characters.

.. note::

    Placing quotation marks around values in a YAML file is generally optional, but values containing wildcard characters do require single or double quotation marks.

Consider the following example:

.. code-block:: yaml

    allowlist:
      - name: AllowedTopic1
        type: Allowed

      - name: AllowedTopic2
        type: "*"

      - name: HelloWorldTopic
        type: HelloWorld

    blocklist:
      - name: "*"
        type: HelloWorld

In this example, the data published in the topic ``AllowedTopic1`` with type ``Allowed`` and in the topic ``AllowedTopic2`` with any type will be recorded by the DDSEnabler.
The data published in the topic ``HelloWorldTopic`` with type ``HelloWorld`` will be blocked, since the ``blocklist`` is blocking all topics with any name and with type ``HelloWorld``.


.. _ddsenabler_topic_qos:

Topic QoS
^^^^^^^^^

The following is the set of QoS that are configurable for a topic.
For more information on topics, please read the `Fast DDS Topic <https://fast-dds.docs.eprosima.com/en/latest/fastdds/dds_layer/topic/topic.html>`_ section.

.. list-table::
    :header-rows: 1

    *   - Quality of Service
        - Yaml tag
        - Data type
        - Default value
        - QoS set

    *   - Reliability
        - ``reliability``
        - *bool*
        - ``false``
        - ``RELIABLE`` / ``BEST_EFFORT``

    *   - Durability
        - ``durability``
        - *bool*
        - ``false``
        - ``TRANSIENT_LOCAL`` / ``VOLATILE``

    *   - Ownership
        - ``ownership``
        - *bool*
        - ``false``
        - ``EXCLUSIVE_OWNERSHIP_QOS`` / ``SHARED_OWNERSHIP_QOS``

    *   - Partitions
        - ``partitions``
        - *bool*
        - ``false``
        - Topic with / without partitions

    *   - Key
        - ``keyed``
        - *bool*
        - ``false``
        - Topic with / without `key <https://fast-dds.docs.eprosima.com/en/latest/fastdds/dds_layer/topic/typeSupport/typeSupport.html#data-types-with-a-key>`_

    *   - History Depth
        - ``history-depth``
        - *unsigned integer*
        - ``5000``
        - :ref:`ddsenabler_history_depth`

    *   - Max Reception Rate
        - ``max-rx-rate``
        - *float*
        - ``0`` (unlimited)
        - :ref:`ddsenabler_max_rx_rate`

    *   - Downsampling
        - ``downsampling``
        - *unsigned integer*
        - ``1``
        - :ref:`ddsenabler_downsampling`

.. warning::

    Manually configuring ``TRANSIENT_LOCAL`` durability may lead to incompatibility issues when the discovered reliability is ``BEST_EFFORT``.
    Please ensure to always configure the ``reliability`` when configuring the ``durability`` to avoid the issue.


.. _ddsenabler_history_depth:

History Depth
"""""""""""""

The ``history-depth`` tag configures the history depth of the Fast DDS internal entities.
By default, the depth of every RTPS History instance is :code:`5000`, which sets a constraint on the maximum number of samples DDSEnabler can deliver to late joiner Readers configured with ``TRANSIENT_LOCAL`` `DurabilityQosPolicyKind <https://fast-dds.docs.eprosima.com/en/latest/fastdds/dds_layer/core/policy/standardQosPolicies.html#durabilityqospolicykind>`_.
Its value should be decreased when the sample size and/or number of created endpoints (increasing with the number of topics) are big enough to cause memory exhaustion issues.
If enough memory is available, however, the ``history-depth`` could be increased to deliver a greater number of samples to late joiners.

.. _ddsenabler_max_rx_rate:

Max Reception Rate
""""""""""""""""""

The ``max-rx-rate`` tag limits the frequency [Hz] at which samples are processed by discarding messages received before :code:`1/max-rx-rate` seconds have passed since the last processed message.
It only accepts non-negative numbers.
By default it is set to ``0``; it processes samples at an unlimited reception rate.

.. _ddsenabler_downsampling:

Downsampling
""""""""""""

The ``downsampling`` tag reduces the sampling rate of the received data by only keeping *1* out of every *n* samples received (per topic), where *n* is the value specified under the ``downsampling`` tag.
When the ``max-rx-rate`` tag is also set, downsampling only applies to messages that have passed the ``max-rx-rate`` filter.
It only accepts positive integers.
By default it is set to ``1``; it accepts every message.

.. _ddsenabler_manual_topics:

Manual Topics
^^^^^^^^^^^^^

A subset of :ref:`Topic QoS <ddsenabler_topic_qos>` can be manually configured for a specific topic under the tag ``topics``.
The tag ``topics`` has a required ``name`` tag that accepts wildcard characters.
It also has two optional tags: a ``type`` tag that accepts wildcard characters, and a ``qos`` tag with the :ref:`Topic QoS <ddsenabler_topic_qos>` that the user wants to manually configure.
If a ``qos`` is not manually configured, it will get its value by discovery.

.. code-block:: yaml

    topics:
      - name: "temperature/*"
        type: "temperature/types/*"
        qos:
          max-rx-rate: 15
          downsampling: 2

.. _ddsenabler_ignore_participant_flags:

Ignore Participant Flags
^^^^^^^^^^^^^^^^^^^^^^^^

A set of discovery traffic filters can be defined in order to add an extra level of isolation.
This configuration option can be set through the ``ignore-participant-flags`` tag:

.. code-block:: yaml

    ignore-participant-flags: no_filter                          # No filter (default)
    # or
    ignore-participant-flags: filter_different_host              # Discovery traffic from another host is discarded
    # or
    ignore-participant-flags: filter_different_process           # Discovery traffic from another process on same host is discarded
    # or
    ignore-participant-flags: filter_same_process                # Discovery traffic from own process is discarded
    # or
    ignore-participant-flags: filter_different_and_same_process  # Discovery traffic from own host is discarded

See `Ignore Participant Flags <https://fast-dds.docs.eprosima.com/en/latest/fastdds/discovery/general_disc_settings.html?highlight=ignore%20flags#ignore-participant-flags>`_ for more information.


.. _ddsenabler_custom_transport_descriptors:

Custom Transport Descriptors
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

By default, DDSEnabler internal participants are created with enabled `UDP <https://fast-dds.docs.eprosima.com/en/latest/fastdds/transport/udp/udp.html>`_ and `Shared Memory <https://fast-dds.docs.eprosima.com/en/latest/fastdds/transport/shared_memory/shared_memory.html>`_ transport descriptors.
The use of one or the other for communication will depend on the specific scenario, and whenever both are viable candidates, the most efficient one (Shared Memory Transport) is automatically selected.
However, a user may desire to force the use of one of the two, which can be accomplished via the ``transport`` configuration tag.

.. code-block:: yaml

    transport: builtin    # UDP & SHM (default)
    # or
    transport: udp        # UDP only
    # or
    transport: shm        # SHM only

.. warning::

    When configured with ``transport: shm``, DDSEnabler will only communicate with applications using Shared Memory Transport exclusively (with disabled UDP transport).


.. _ddsenabler_interface_whitelist:

Interface Whitelist
^^^^^^^^^^^^^^^^^^^

Optional tag ``whitelist-interfaces`` allows to limit the network interfaces used by UDP and TCP transport.
This may be useful to only allow communication within the host (note: same can be done with :ref:`ddsenabler_ignore_participant_flags`).
Example:

.. code-block:: yaml

    whitelist-interfaces:
      - "127.0.0.1"    # Localhost only

See `Interface Whitelist <https://fast-dds.docs.eprosima.com/en/latest/fastdds/transport/whitelist.html>`_ for more information.


.. _ddsenabler_ddsenabler_configuration:

DDSEnabler Configuration
-----------------------

Configuration of data writing to the Context Broker.

.. _ddsenabler_specs_configuration:

Specs Configuration
-------------------

The internals of DDSEnabler can be configured using the ``specs`` optional tag that contains certain options related with the overall configuration of the DDSEnabler instance to run.
The values available to configure are:

.. _ddsenabler_threads:

Number of Threads
^^^^^^^^^^^^^^^^^

``specs`` supports a ``threads`` optional value that allows the user to set a maximum number of threads for the internal :code:`ThreadPool`.
This ThreadPool allows to limit the number of threads spawned by the application.
This improves the performance of the internal data communications.

This value should be set by each user depending on each system characteristics.
In case this value is not set, the default number of threads used is :code:`12`.

.. _ddsenabler_specs_logging:

Logging
^^^^^^^

``specs`` supports a ``logging`` **optional** tag to configure the DDSEnabler logs.
Under the ``logging`` tag, users can configure the type of logs to display and filter the logs based on their content and category.
When configuring the verbosity to ``info``, all types of logs, including informational messages, warnings, and errors, will be displayed.
Conversely, setting it to ``warning`` will only show warnings and errors, while choosing ``error`` will exclusively display errors.
By default, the filter allows all errors to be displayed, while selectively permitting warning and informational messages from ``DDSENABLER`` category.

.. list-table::
    :header-rows: 1

    *   - Logging
        - Yaml tag
        - Description
        - Data type
        - Default value
        - Possible values

    *   - Verbosity
        - ``verbosity``
        - Show messages of equal |br|
          or higher importance.
        - *enum*
        - ``error``
        - ``info`` / ``warning`` / ``error``

    *   - Filter
        - ``filter``
        - Regex to filter the category  |br|
          or message of the logs.
        - *string*
        - info : ``DDSENABLER`` |br|
          warning : ``DDSENABLER`` |br|
          error : ``""``
        - Regex string

.. note::

    For the logs to function properly, the ``-DLOG_INFO=ON`` compilation flag is required.

The DDSEnabler prints the logs by default (warnings and errors in the standard error and infos in the standard output).
The DDSEnabler, however, can also publish the logs in a DDS topic.
To publish the logs, under the tag ``publish``, set ``enable: true`` and set a ``domain`` and a ``topic-name``.
The type of the logs published is defined as follows:

**LogEntry.idl**

.. code-block:: idl

    const long UNDEFINED = 0x10000000;
    const long SAMPLE_LOST = 0x10000001;
    const long TOPIC_MISMATCH_TYPE = 0x10000002;
    const long TOPIC_MISMATCH_QOS = 0x10000003;
    const long FAIL_MCAP_CREATION = 0x12000001;
    const long FAIL_MCAP_WRITE = 0x12000002;

    enum Kind {
      Info,
      Warning,
      Error
    };

    struct LogEntry {
      @key long event;
      Kind kind;
      string category;
      string message;
      string timestamp;
    };

.. note::

    The type of the logs can be published by setting ``publish-type: true``.

**Example of usage**

.. code-block:: yaml

    logging:
      verbosity: info
      filter:
        error: "DDSPIPE|DDSENABLER"
        warning: "DDSPIPE|DDSENABLER"
        info: "DDSENABLER"
      publish:
        enable: true
        domain: 84
        topic-name: "DDSEnablerLogs"
        publish-type: false
      stdout: true

.. _ddsenabler_usage_configuration_general_example:

General Example
---------------

A complete example of all the configurations described on this page can be found below.

.. warning::

    This example can be used as a quick reference, but it may not be correct due to incompatibility or exclusive properties. **Do not take it as a working example**.

.. code-block:: yaml

    dds:
      domain: 0

      allowlist:
        - name: "topic_name"
          type: "topic_type"

      blocklist:
        - name: "topic_name"
          type: "topic_type"

      builtin-topics:
        - name: "HelloWorldTopic"
          type: "HelloWorld"

      topics:
        - name: "temperature/*"
          type: "temperature/types/*"
          qos:
            max-rx-rate: 15
            downsampling: 2

      ignore-participant-flags: no_filter
      transport: builtin
      whitelist-interfaces:
        - "127.0.0.1"

    ddsenabler:

    specs:
      threads: 8

      logging:
        verbosity: info
        filter:
          error: "DDSPIPE|DDSENABLER"
          warning: "DDSPIPE|DDSENABLER"
          info: "DDSENABLER"
        publish:
          enable: true
          domain: 84
          topic-name: "DDSEnablerLogs"
          publish-type: false
        stdout: true
