# SOTP

A lightweight module providing the Soft One Time Programming functionality

## Description

SOTP provides the ability to store a very minimal set of system critical items in the internal flash.
For each item type, SOTP module provides the ability to set the item data or get it.
Newly set values are written to the end of the flash area, overriding the previous value for this item.
SOTP module makes sure that programmed data sustains power failures.
The full interface can be found under sotp.h.

### Flash structure
SOTP uses two Flash areas, active and non-active. Data is written to the active area, until it gets full.
When it does, garbage collection is invoked, compacting items from the active area to the non-active one,
and switching activity between areas.
Each item is kept in an entry, containing header and data, where the header holds the item type, size and MAC.
Each area starts with a master record, whose purpose is to help us figure out the flash status at init time.

## Usage

### Enabling SOTP feature

Add the following to your mbed_app.json:

```json
{
    "target_overrides": {
        "*": {
            "target.features_add": ["SOTP"],
        }
    }
}
```

### Configuring SOTP for your board

Set the addresses and sizes of both flash areas in mbed_lib.json file. num_areas should be set to 2.
In addition, SOTP is by default not thread safe. To achieve thread saftey,
set the SOTP_THREAD_SAFE flag in this json file to 1.

### Building SOTP
To reduce code size, make sure SOTP_TESTING compilation flag is not set.
Minimal applications (such as boot loaders), requiring a read only access to the SOTP area, can build SOTP with
the SOTP_PROBE_ONLY compilation flag set. This will aggressively minimize code size by providing the
sotp_probe API only.

### Using SOTP
SOTP is a singleton class, meaning that the application can have only a single instance of it.
To instanciate SOTP, one needs to call its get_instance member function as following:
    SOTP &sotp = SOTP::get_instance();
By default, SOTP uses the SOTP_MAX_TYPE definition as its number of types. One can call the set_num_types
API (preferably before first usage) to change it.
After the SOTP instantiation, one can call the init API, but it is not necessary, as all
SOTP APIs (get, set et al.) perform a "lazy initialization".

### Testing SOTP
Run the SOTP functionality test with the mbed command as following:
mbed test -n features-feature_sotp-tests-sotp-functionality