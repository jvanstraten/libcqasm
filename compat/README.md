# Compatibility layer

These files provide the exact same interface as libqasm used to, for
compatibility purposes. Their implementation is modified to be a header-only
wrapper around the new library, so bugfixes and (compatible) additions made
in the new library translate into the old API as much as possible.

Given the choice, however, please use the new API. The old API leaks memory
by design, and its exception mechanism is very confused (most of the time,
the return codes can only ever be success, because exceptions prevent them
from being set to failure), to name a few of the many things wrong with
it.
