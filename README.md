# ESPHome Integration for Daikin BRC52B61/62/63/64/65 Remote Control

This project forks the [ESPHome Daikin component](
https://github.com/esphome/esphome/tree/c61a3bf4314606119f1c4e1771cd9c234000b4ed/esphome/components/daikin
) to support the IR protocol used by the BRC52B6X family of remote controls,
which control some Daikin heat pumps and air conditioners. The protocol is
tested with an FTXB/FTKB-series air handler and may work with some other models.

You can import the new components into your ESPHome config directly from this
repository:

```
external_components:
  - source: github://jseyster/esphome-daikin-brc52b6x@master
    components: [ daikin_brc52b6x ]
```

## Configuration

Configuration is the same as for other IR-controlled climate components:
https://esphome.io/components/climate/climate_ir.html

Optional support for an IR receiver, which allows the component to update its
state when it observes commands sent by other remotes, is included.

You can also (optionally) specify a time source to use when updating the
controlled unit's clock. Without a time source, each command that ESPHome sends
to the unit will roll its clock back to midnight. The clock only matters if you
use the unit's internal on/off timers, however.

```
time:
  - platform: homeassistant
    id: homeassistant_time

climate:
  - platform: daikin_brc52b6x
    id: daikin_hvac
    name: HVAC
    time_id: homeassistant_time
```

## Notes

BRC52B6X remotes do not have discrete, idempotent on and off buttons, only a
power toggle button. If the protocol has on and off commands, they are well
hidden. I cannot fathom why the developers chose not to include such obvious and
basic functionality, but without it, this component can end up out of sync with
the unit, turning it on when it wants it to be off and vice versa.

One way you may end up in this state is after restarting ESPHome. I haven't yet
implemented persistance for the unit's on/off state, so ESPHome will always
assume that the unit is off when it first boots up.

## License

This component is forked from the original Daikin component in ESPHome and
shares its licensing: the GPL version 3 for C++ code and the MIT license for
Python code.
