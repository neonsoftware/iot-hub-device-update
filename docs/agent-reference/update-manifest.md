# ADU Update Manifest

## Overview

ADU communicates actions and metadata supporting actions (a.k.a. the Update Manifest) through the
 [AzureDeviceUpdateCore.OrchestratorMetadata:4](./../how-adu-uses-iot-pnp.md) interface properties.
 This document describes the fundamentals of how the `updateManifest` property, in the
 `AzureDeviceUpdateCore.OrchestratorMetadata:4` interface, is used by the ADU Agent. The
 `AzureDeviceUpdateCore.OrchestratorMetadata:4` interface properties are sent from the ADU Service
 to the ADU Agent. The `updateManifest` is a serialized JSON Object that is parsed by the ADU Agent.

### Example Update Manifest

```JSON
{
    "manifestVersion": "1",
    "updateId": {
        "provider": "AduTest",
        "name": "AduTestUser",
        "version": "2020.611.534.16"
    },
    "updateType": "microsoft/swupdate:1",
    "installedCriteria": "1.0",
    "files": {
        "00000": {
            "fileName": "image.swu",
            "sizeInBytes": 256000,
            "hashes": {
                "sha256": "IhIIxBJpLfazQOk/PVi6SzR7BM0jf4HDqw+6gdZ3vp8="
            }
        }
    },
    "createdDateTime": "2020-06-12T00:38:13.9350278"
}
```

The purpose of the Update Manifest is to describe the contents of an update, namely its identity, type,
installed criteria, and update file metadata. In addition, the Update Manifest is cryptographically signed to
allow the ADU Agent to verify its authenticity.

### Validating Update Manifest Security

The Update Manifest is validated through the use of two signatures. The signatures are created using a structure consisting of *signing* keys and *root* keys.

The ADU Agent has embedded public keys which are used for any ADU-compatible device. These are the *root* keys. The corresponding private keys are controlled by Microsoft.

Microsoft also generates a public/private key pair which is not included in the ADU Agent or stored on the device. This is the *signing* key.

When an update is imported into ADU and the Update Manifest is generated by the service, the service signs the manifest using the signing key, and includes the signing key itself which is signed by a root key. When the Update Manifest is sent to the device, the ADU Agent receives the following signature data:

1. The signature value itself.
2. The algorithm used for generating #1.
3. The public key information of the signing key used for generating #1.
4. The signature of the public signing key in #3.
5. The public key ID of the root key used for generating #3.
6. The algorithm used for generating #4.

The ADU Agent uses the information defined above to validate that signature of the public signing key is signed by the root key. The ADU Agent then validates that the Update Manifest signature is signed by the signing key. If all the signatures are correct, the Update Manifest is trusted by the ADU Agent. Since the Update Manifest includes the file hashes which correspond to the update files themselves, the update files can then also be trusted if the hashes match.

Having root and signing keys allows us to periodically roll the signing key, a security best practice.

### JSON Web Signature (JWS)

The `updateManifestSignature` is used to ensure that the information contained within the `updateManifest` has
not been tampered with. The `updateManifestSignature` is produced using a JSON Web Signature with JSON Web Keys, allowing for source verification. The signature is a Base64Url Encoded string with three sections delineated by ".".  Refer to the [jws_util.h](../../src/utils/jws_utils/inc/jws_utils.h) helper methods for parsing and verifying JSON keys and tokens.

JSON Web Signature is a widely used [proposed IETF standard](https://tools.ietf.org/html/rfc7515) for signing
content using JSON-based data structures. It is a way of ensuring integrity of data by verifying the signature
of the data. Further information can be found in the JSON Web Signature (JWS) [RFC 7515](https://www.rfc-editor.org/info/rfc7515).

### JSON Web Token

JSON Web Tokens are an open, industry [standard](https://tools.ietf.org/html/rfc7519) method for representing
claims securely between two parties.

## Import Manifest vs Update Manifest

It is important to understand the differences between the Import Manifest and the Update Manifest concepts in ADU. The Import Manifest
is created by update creators to describe the contents of the update that will be imported into ADU. The Update Manifest
is created by the ADU Service, using some of the properties that were defined in the Import Manifest, to communicate
relevant information to the ADU Agent upon an update. Each manifest type has it's own schema and schema version.
This file is focused on the details of the Update Manifest. [Learn more](./../quickstarts/how-to-import-quickstart.md)
about the details of the Import Manifest.

## Update Manifest Properties

The high-level definitions of the Update Manifest properties can be found in the interface definitions found
[here](./../how-adu-uses-iot-pnp.md). To provide deeper context, let's take a closer look
at the properties and how they are used in the system.

### updateId

Contains the `provider`, `name`, and `version`, which represents the exact ADU update identity used
to determine compatible devices for the update.

### updateType

Represents the type of update which is handled by a specific type of update handler. It follows the form
of `microsoft/swupdate:1` for an image-based update and `microsoft/apt:1` for a package-based update (see `Update Handler Types` section below).

### installedCriteria

A string that contains information needed by ADU Agent's Update Handler to determine whether the update is
installed on the device. The `Update Handler Types` section documents the format of the `installedCriteria`,
for each update type supported by ADU.

### files

Tells the ADU Agent which files to download and the hash which will be used to use to verify the files were downloaded correctly.
Here's a closer look at the `files` property contents:

```json
"files":{
        <FILE_ID_STRING>:{
            "fileName":<STRING>,
            "sizeInBytes":<INTEGER>,
            "hashes":{
                <HASH-TYPE>:<HASH-STRING>
            }
        }
    }
```

Outside of the `updateManifest` is the `fileUrls` array of JSON Object.

```json
"fileUrls":{
      <FILE_ID_STRING>: <URL-in-String-Format>
 }
```

Both the `FILE_ID_STRING`, within `fileUrls`, and `files` are the same (e.g. "0000" in `files` has the url
at "0000" within `fileUrls`).

### manifestVersion

A string that represents the schema version.

## Update Handler Types

|Update Method|Update Handler Type|Update Type|Installed Criteria|Expected Files for Publishing|
|-------------|-------------------|----------|-----------------|--------------|
|Image-based|SWUpdate|"microsoft/swupdate:version"|`<ADU_SOFTWARE_VERSION>` (defined in the Yocto build configuration [here](./yocto-configuration.md))|.swu file which contains SWUpdate image|
|Package-based|APT|"microsoft/apt:version"|`<name>` + "-" + `<version>` (defined properties in the APT Manifest file|`<APT Update Manifest>`.json which contains the APT configuration and package list|

**Note**: The `ADU_SOFTWARE_VERSION` will be saved to the `/etc/adu-version` file.