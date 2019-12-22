Linux containers from scratch using namespaces and cgroups

(work in progress)

current functionality includes enforcing memory and process limits.

assumes that a copy of the file system exists at ```~/rootfs/``` (run ```createFS.sh``` to make create one, but requires some more changes as specified in the file).
execution:
```gcc cntrs.c```

```sudo ./a.out bash```
