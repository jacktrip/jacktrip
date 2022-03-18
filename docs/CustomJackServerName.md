# Using custom JACK server name

> ❕ This note is applicable only if you use JACK backend.

In case you want to use JackTrip with JACK server _that has non-default name_ (by default it is `default`), you need to set `JACK_DEFAULT_SERVER` environment variable for JackTrip.

1. Run `jackd` with `--name` flag:`jackd --name myfancyserver ...`,
2. Run JackTrip with required environment variable set: `JACK_DEFAULT_SERVER=myfancyserver jacktrip ...`.

This is useful when you want to isolate multiple JackTrip instances on single machine.
