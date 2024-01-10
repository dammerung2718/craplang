# CrapLang (Cross-platform Application Language)

Okay so we have to figure this out.

What do we need?

- A view would be nice.
- Some kind of place to put what kind of events this app produces so we can fucking store them.
- A way to define business logic?

What is at heart of the project?

- UI = f(state)
- state = sum(events)
- event = ??? (depends on the application lmao)

what is the common thing between all of these equations?

THEY ARE FUCKING STATELESS.

aight lets do this shit.


## STEPS TO PRODUCE THIS SHITTY THING.

We need a window people can use.  Let's fuck ourselves 10x less and
restrict platform support to macOS initially.

We will also generate Node.js for backend so that kinda makes things
less suck (for us, not particularly to the user).
