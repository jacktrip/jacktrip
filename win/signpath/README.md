# SignPath artifact configurations

The Windows installer is signed by [SignPath Foundation](https://signpath.org) in the
`sign-windows` job of `.github/workflows/jacktrip.yml`. SignPath uses its own stored copy
of each artifact configuration; the files here are the reference copies and must be kept
in sync with the SignPath project by hand.

A WiX Burn bundle has to be signed in three steps, so there is one configuration (and one
signing request) for each:

| Slug                   | File                       | Signs                                                      |
|------------------------|----------------------------|------------------------------------------------------------|
| `installer-components` | `installer-components.xml` | `jacktrip.exe` and the VST3 inside `JackTrip.msi`, the MSI, `uninstall_old_jacktrip.exe` |
| `burn-engine`          | `burn-engine.xml`          | Burn engine detached from the bundle (`insignia -ib`)      |
| `bundle`               | `bundle.xml`               | `JackTrip-Setup.exe` after the signed engine is reattached (`insignia -ab`) |

The workflow expects these repository variables:

- `SIGNPATH_ORGANIZATION_ID`
- `SIGNPATH_PROJECT_SLUG`
- `SIGNPATH_SIGNING_POLICY_SLUG`

and the `SIGNPATH_API_TOKEN` secret in the `signing` environment.
