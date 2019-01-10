/* Generated by the protocol buffer compiler.  DO NOT EDIT! */
/* Generated from: acl.proto */

#ifndef PROTOBUF_C_acl_2eproto__INCLUDED
#define PROTOBUF_C_acl_2eproto__INCLUDED

#include <protobuf-c/protobuf-c.h>

PROTOBUF_C__BEGIN_DECLS

#if PROTOBUF_C_VERSION_NUMBER < 1003000
# error This file was generated by a newer version of protoc-c which is incompatible with your libprotobuf-c headers. Please update your headers.
#elif 1003001 < PROTOBUF_C_MIN_COMPILER_VERSION
# error This file was generated by an older version of protoc-c which is incompatible with your libprotobuf-c headers. Please regenerate this file with a newer version of protoc-c.
#endif


typedef struct _Proto__AclResponse Proto__AclResponse;
typedef struct _Proto__AclPrincipal Proto__AclPrincipal;
typedef struct _Proto__AclEntry Proto__AclEntry;
typedef struct _Proto__AclEntryPermissions Proto__AclEntryPermissions;


/* --- enums --- */

typedef enum _Proto__AclRequestStatus {
  PROTO__ACL_REQUEST_STATUS__SUCCESS = 0,
  /*
   * Unknown error
   */
  PROTO__ACL_REQUEST_STATUS__ERR_UNKNOWN = -1,
  /*
   * Not authorized to make these changes
   */
  PROTO__ACL_REQUEST_STATUS__ERR_PERM_DENIED = -2,
  /*
   * Permissions requested are invalid
   */
  PROTO__ACL_REQUEST_STATUS__ERR_INVALID_PERMS = -3,
  /*
   * Principal requested is invalid
   */
  PROTO__ACL_REQUEST_STATUS__ERR_INVALID_PRINCIPAL = -4,
  /*
   * UUID requested is invalid
   */
  PROTO__ACL_REQUEST_STATUS__ERR_INVALID_UUID = -5
    PROTOBUF_C__FORCE_ENUM_TO_BE_INT_SIZE(PROTO__ACL_REQUEST_STATUS)
} Proto__AclRequestStatus;
/*
 * Bits representing access permissions
 */
typedef enum _Proto__AclPermissions {
  PROTO__ACL_PERMISSIONS__NONE = 0,
  PROTO__ACL_PERMISSIONS__READ = 1,
  PROTO__ACL_PERMISSIONS__WRITE = 2
    PROTOBUF_C__FORCE_ENUM_TO_BE_INT_SIZE(PROTO__ACL_PERMISSIONS)
} Proto__AclPermissions;
/*
 * Types of principals that can have access
 */
typedef enum _Proto__AclPrincipalType {
  PROTO__ACL_PRINCIPAL_TYPE__USER = 0,
  PROTO__ACL_PRINCIPAL_TYPE__GROUP = 1,
  PROTO__ACL_PRINCIPAL_TYPE__EVERYONE = 2
    PROTOBUF_C__FORCE_ENUM_TO_BE_INT_SIZE(PROTO__ACL_PRINCIPAL_TYPE)
} Proto__AclPrincipalType;

/* --- messages --- */

struct  _Proto__AclResponse
{
  ProtobufCMessage base;
  Proto__AclRequestStatus status;
  Proto__AclEntryPermissions *permissions;
};
#define PROTO__ACL_RESPONSE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&proto__acl_response__descriptor) \
    , PROTO__ACL_REQUEST_STATUS__SUCCESS, NULL }


typedef enum {
  PROTO__ACL_PRINCIPAL__IDENTIFIER__NOT_SET = 0,
  PROTO__ACL_PRINCIPAL__IDENTIFIER_ID = 2,
  PROTO__ACL_PRINCIPAL__IDENTIFIER_NAME = 3
    PROTOBUF_C__FORCE_ENUM_TO_BE_INT_SIZE(PROTO__ACL_PRINCIPAL__IDENTIFIER)
} Proto__AclPrincipal__IdentifierCase;

/*
 * Represents a user or group who could access an entity.
 * If it is "everyone" we can ignore the identifying information.
 */
struct  _Proto__AclPrincipal
{
  ProtobufCMessage base;
  Proto__AclPrincipalType type;
  Proto__AclPrincipal__IdentifierCase identifier_case;
  union {
    /*
     * uid or gid
     */
    uint32_t id;
    char *name;
  };
};
#define PROTO__ACL_PRINCIPAL__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&proto__acl_principal__descriptor) \
    , PROTO__ACL_PRINCIPAL_TYPE__USER, PROTO__ACL_PRINCIPAL__IDENTIFIER__NOT_SET, {0} }


/*
 * Identifier for an Access Control Entry
 */
struct  _Proto__AclEntry
{
  ProtobufCMessage base;
  /*
   * UUID of the entity (such as a pool)
   */
  char *uuid;
  /*
   * User/group who accesses the entity
   */
  Proto__AclPrincipal *principal;
};
#define PROTO__ACL_ENTRY__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&proto__acl_entry__descriptor) \
    , (char *)protobuf_c_empty_string, NULL }


/*
 * Permissions for the given entry
 */
struct  _Proto__AclEntryPermissions
{
  ProtobufCMessage base;
  Proto__AclEntry *entry;
  /*
   * Bitmask of AclPermissions
   */
  uint32_t permission_bits;
};
#define PROTO__ACL_ENTRY_PERMISSIONS__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&proto__acl_entry_permissions__descriptor) \
    , NULL, 0 }


/* Proto__AclResponse methods */
void   proto__acl_response__init
                     (Proto__AclResponse         *message);
size_t proto__acl_response__get_packed_size
                     (const Proto__AclResponse   *message);
size_t proto__acl_response__pack
                     (const Proto__AclResponse   *message,
                      uint8_t             *out);
size_t proto__acl_response__pack_to_buffer
                     (const Proto__AclResponse   *message,
                      ProtobufCBuffer     *buffer);
Proto__AclResponse *
       proto__acl_response__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   proto__acl_response__free_unpacked
                     (Proto__AclResponse *message,
                      ProtobufCAllocator *allocator);
/* Proto__AclPrincipal methods */
void   proto__acl_principal__init
                     (Proto__AclPrincipal         *message);
size_t proto__acl_principal__get_packed_size
                     (const Proto__AclPrincipal   *message);
size_t proto__acl_principal__pack
                     (const Proto__AclPrincipal   *message,
                      uint8_t             *out);
size_t proto__acl_principal__pack_to_buffer
                     (const Proto__AclPrincipal   *message,
                      ProtobufCBuffer     *buffer);
Proto__AclPrincipal *
       proto__acl_principal__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   proto__acl_principal__free_unpacked
                     (Proto__AclPrincipal *message,
                      ProtobufCAllocator *allocator);
/* Proto__AclEntry methods */
void   proto__acl_entry__init
                     (Proto__AclEntry         *message);
size_t proto__acl_entry__get_packed_size
                     (const Proto__AclEntry   *message);
size_t proto__acl_entry__pack
                     (const Proto__AclEntry   *message,
                      uint8_t             *out);
size_t proto__acl_entry__pack_to_buffer
                     (const Proto__AclEntry   *message,
                      ProtobufCBuffer     *buffer);
Proto__AclEntry *
       proto__acl_entry__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   proto__acl_entry__free_unpacked
                     (Proto__AclEntry *message,
                      ProtobufCAllocator *allocator);
/* Proto__AclEntryPermissions methods */
void   proto__acl_entry_permissions__init
                     (Proto__AclEntryPermissions         *message);
size_t proto__acl_entry_permissions__get_packed_size
                     (const Proto__AclEntryPermissions   *message);
size_t proto__acl_entry_permissions__pack
                     (const Proto__AclEntryPermissions   *message,
                      uint8_t             *out);
size_t proto__acl_entry_permissions__pack_to_buffer
                     (const Proto__AclEntryPermissions   *message,
                      ProtobufCBuffer     *buffer);
Proto__AclEntryPermissions *
       proto__acl_entry_permissions__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   proto__acl_entry_permissions__free_unpacked
                     (Proto__AclEntryPermissions *message,
                      ProtobufCAllocator *allocator);
/* --- per-message closures --- */

typedef void (*Proto__AclResponse_Closure)
                 (const Proto__AclResponse *message,
                  void *closure_data);
typedef void (*Proto__AclPrincipal_Closure)
                 (const Proto__AclPrincipal *message,
                  void *closure_data);
typedef void (*Proto__AclEntry_Closure)
                 (const Proto__AclEntry *message,
                  void *closure_data);
typedef void (*Proto__AclEntryPermissions_Closure)
                 (const Proto__AclEntryPermissions *message,
                  void *closure_data);

/* --- services --- */

typedef struct _Proto__AccessControl_Service Proto__AccessControl_Service;
struct _Proto__AccessControl_Service
{
  ProtobufCService base;
  void (*set_permissions)(Proto__AccessControl_Service *service,
                          const Proto__AclEntryPermissions *input,
                          Proto__AclResponse_Closure closure,
                          void *closure_data);
  void (*get_permissions)(Proto__AccessControl_Service *service,
                          const Proto__AclEntry *input,
                          Proto__AclResponse_Closure closure,
                          void *closure_data);
  void (*destroy_acl_entry)(Proto__AccessControl_Service *service,
                            const Proto__AclEntry *input,
                            Proto__AclResponse_Closure closure,
                            void *closure_data);
};
typedef void (*Proto__AccessControl_ServiceDestroy)(Proto__AccessControl_Service *);
void proto__access_control__init (Proto__AccessControl_Service *service,
                                  Proto__AccessControl_ServiceDestroy destroy);
#define PROTO__ACCESS_CONTROL__BASE_INIT \
    { &proto__access_control__descriptor, protobuf_c_service_invoke_internal, NULL }
#define PROTO__ACCESS_CONTROL__INIT(function_prefix__) \
    { PROTO__ACCESS_CONTROL__BASE_INIT,\
      function_prefix__ ## set_permissions,\
      function_prefix__ ## get_permissions,\
      function_prefix__ ## destroy_acl_entry  }
void proto__access_control__set_permissions(ProtobufCService *service,
                                            const Proto__AclEntryPermissions *input,
                                            Proto__AclResponse_Closure closure,
                                            void *closure_data);
void proto__access_control__get_permissions(ProtobufCService *service,
                                            const Proto__AclEntry *input,
                                            Proto__AclResponse_Closure closure,
                                            void *closure_data);
void proto__access_control__destroy_acl_entry(ProtobufCService *service,
                                              const Proto__AclEntry *input,
                                              Proto__AclResponse_Closure closure,
                                              void *closure_data);

/* --- descriptors --- */

extern const ProtobufCEnumDescriptor    proto__acl_request_status__descriptor;
extern const ProtobufCEnumDescriptor    proto__acl_permissions__descriptor;
extern const ProtobufCEnumDescriptor    proto__acl_principal_type__descriptor;
extern const ProtobufCMessageDescriptor proto__acl_response__descriptor;
extern const ProtobufCMessageDescriptor proto__acl_principal__descriptor;
extern const ProtobufCMessageDescriptor proto__acl_entry__descriptor;
extern const ProtobufCMessageDescriptor proto__acl_entry_permissions__descriptor;
extern const ProtobufCServiceDescriptor proto__access_control__descriptor;

PROTOBUF_C__END_DECLS


#endif  /* PROTOBUF_C_acl_2eproto__INCLUDED */
