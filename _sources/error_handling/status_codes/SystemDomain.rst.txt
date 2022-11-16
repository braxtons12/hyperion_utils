SystemDomain
************

.. doxygendefine:: HYPERION_WINDOWS_USES_POSIX_CODES

:cpp:`HYPERION_SYSTEM_STATUS_CODE_NOT_POSIX`

Configures Hyperion to __NOT__ use :cpp:`hyperion::error::PosixDomain` as the system
:cpp:`StatusCodeDomain`.

To enable this configuration, define :cpp:`HYPERION_SYSTEM_STATUS_CODE_NOT_POSIX` to any value
Normally, this would only be used on systems that do not support POSIX, but can also be used
to disable it on systems that would normally use POSIX compliant error codes (:cpp:`errno`),
but shouldn't for your use case. In either situation, you __MUST__ define
:cpp:`HYPERION_SYSTEM_STATUS_CODE_DOMAIN` to the type of your desired :cpp:`StatusCodeDomain`
prior to including __any__ Hyperion headers other than :cpp:`<Hyperion/error/StatusCodeDomain.h>`

.. doxygendefine:: HYPERION_SYSTEM_STATUS_CODE_DOMAIN
.. doxygentypedef:: hyperion::error::SystemDomain
.. doxygentypedef:: hyperion::error::SystemStatusCode
.. doxygentypedef:: hyperion::error::SystemErrorCode
.. doxygentypedef:: hyperion::error::SystemError

.. doxygenconcept:: hyperion::error::IsStatusCode
