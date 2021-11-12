## TODO

### NEXT STEPS

### IMPORTANT
- memory: plan for total memory release (pool, etc...) to please valgrind.
    - not so easy: need to keep track of allocated blocks, **and** to understand that no object is in use.
- move.c, piece.c: function to remove an element from list.
- position.c: duplicate fully a position (including board & piece list)
    - should move list be duplicated ?

### MISC
- `move_gen()`: `doit` is maybe redundant with test on current turn.
- should board be included in position ?
