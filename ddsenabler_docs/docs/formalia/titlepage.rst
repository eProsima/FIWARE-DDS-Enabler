
What is DDS Enabler?
^^^^^^^^^^^^^^^^^^^^

.. image:: /_static/eprosima-logo.svg
  :height: 100px
  :width: 100px
  :align: left
  :alt: eProsima
  :target: http://www.eprosima.com/


**eProsima DDS Enabler** is a modular middleware solution enabling seamless integration between distributed systems by
building on top of the Data Distribution Service (DDS) standard. It allows developers to create scalable, reliable, and
real-time communication infrastructures with minimal effort.

As part of the ARISE project, DDS Enabler has been developed to facilitate OT/IT convergence by bridging robotic (ROS 2)
and information systems (NGSI-LD context brokers), enabling a unified communication framework for industrial environments.
Its foundation on Fast DDS guarantees low-latency, high-performance capabilities suitable for critical HRI scenarios.

.. raw:: html

    <br/>
    
Commercial support
^^^^^^^^^^^^^^^^^^

Looking for commercial support? Write us at **info@eprosima.com**.

Find more about us at `eProsima's webpage <https://eprosima.com/>`_.

Overview
^^^^^^^^

*DDS Enabler* is one of the technical cornerstones in the ARISE project's vision for an all-in-one middleware
enabling **real-time industrial human-robot interaction**. Designed to unify **Operational Technologies (OT)** and **Information Technologies (IT)**, it is a modular and open solution that integrates natively with **Fast DDS** and **NGSI-LD**.

Key characteristics include:

- **Real-time publish-subscribe middleware**: Built on the DDS standard, enabling low-latency and reliable communication.
- **Seamless OT/IT integration**: Acts as a translator between DDS-based systems and NGSI-LD context brokers.
- **ROS 2 and FIWARE compatibility**: Enables industrial robots and context-aware applications to operate through the same infrastructure.
- **Scalable and extensible architecture**: Fully compatible with modern I4.0 deployments, providing flexibility for growth and adaptation.

In the ARISE framework, DDS Enabler serves as the bridge enabling *read and write* interactions between DDS and
HTTP-based systems. This includes translating DDS topic samples into NGSI-LD entities, and vice versa, enabling
**bidirectional interoperability** between robot data and smart industry platforms.

Structure of the documentation
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

This documentation is organized into the following main sections:

* :ref:`Installation Manual <installation_manual_linux>`
* :ref:`Project Overview <project_overview>`
* :ref:`Configuration <configuration>`
* :ref:`Developer Manual <developer_manual_installation_sources_linux>`
* :ref:`API Reference <api_reference>`
* :ref:`Release Notes <release_notes>`
