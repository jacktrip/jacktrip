# Running multiple instances

If you want to setup multiple JackTrip instances on single machine, you have to use _different JACK servers_. Set `--name` flag for `jackd` flag and accordingly set `JACK_DEFAULT_SERVER` environment variable for JackTrip instances.

For example:

- First instance:
  `jackd -n server_1 ...` and `JACK_DEFAULT_SERVER=server_1 jacktrip ...`
- Second instance:
  `jackd -n server_2 ...` and `JACK_DEFAULT_SERVER=server_2 jacktrip ...`
