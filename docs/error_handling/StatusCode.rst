StatusCode
**********

.. doxygenconcept:: hyperion::error::UUIDString

.. doxygenvariable:: hyperion::error::num_chars_in_uuid

.. doxygenvariable:: hyperion::error::num_chars_in_ms_uuid

.. doxygenconcept:: hyperion::error::StatusCodeDomain

.. doxygenfunction:: hyperion::error::parse_uuid_from_string

.. doxygendefine:: STATUS_CODE_DOMAIN

.. doxygenstruct:: hyperion::error::status_code_enum_info
   :members:
   :undoc-members:

.. doxygenstruct:: hyperion::error::is_status_code_enum
   :members:
   :undoc-members:

.. doxygenvariable:: hyperion::error::is_status_code_enum_v

.. doxygenconcept:: hyperion::error::StatusCodeEnum

.. doxygentypedef:: hyperion::error::status_code_enum_domain

.. doxygentypedef:: hyperion::error::status_code_enum_domain_checked

.. doxygenconcept:: hyperion::error::ConvertibleToGenericStatusCode

.. doxygenclass:: hyperion::error::StatusCode
   :members:

.. doxygenfunction:: make_status_code(hyperion::error::StatusCodeEnum auto)
.. doxygenfunction:: make_status_code(hyperion::i64)
.. doxygenfunction:: make_status_code(hyperion::i64, Domain&&)

.. doxygenclass:: hyperion::error::ErrorCode
   :members:

.. doxygenfunction:: make_error_code(hyperion::error::StatusCodeEnum auto)
.. doxygenfunction:: make_error_code(hyperion::i64)
.. doxygenfunction:: make_error_code(hyperion::i64, Domain&&)

