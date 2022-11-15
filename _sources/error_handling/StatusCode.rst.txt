StatusCode
**********

Overview
--------

This module provides Hyperion's basic low-level functionality for representing errors.
This functionality includes defining the basic requirements for status code domain types,
status codes, and the machinery for working with them.

Status code domains (in code, :cpp:`StatusCodeDomain` s) represent domains, or categories, from which
potential errors can occur. For example, :cpp:`PosixDomain`, :cpp:`Win32Domain`, and :cpp:`NTDomain`
are all :cpp:`StatusCodeDomain` s that represent errors that can occur from POSIX functions and
Windows's Win32 and NT subsystems, respectively. :cpp:`StatusCodeDomain` s provide the functionality
to convert from low-level error codes from their respective domain to textual error messages those
codes represent, check whether a given code represents success or failure, and convert a code from
its domain to the :cpp:`GenericDomain`'s :cpp:`hyperion::error::Errno` (if applicable).

.. toctree::
    :caption: Modules

    status_codes/StatusCodeDomain
    status_codes/StatusCode


