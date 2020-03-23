# Communication protocol between front-end and back-end

## Setting update

Setting update should happen when the front-end first connect to the back-end
and when the user choose to change the ray distribution.
Each ray is represented by two floating points x and y representing a ray
vector `<x, y, -1>` in view space.

### front -> back

```text
0       1       2       3       4       5       6       7
+-------------------------------+-------------------------------+
|        Trace depth (32)       |       Number of rays (32)     |
+-------------------------------+-------------------------------+
|         x of ray 1 (32)       |         y of ray 1 (32)       |
+-------------------------------+-------------------------------+
|         x of ray 2 (32)       |         y of ray 2 (32)       |
+-------------------------------+-------------------------------+
|                       Rays continued ...                      |
+---------------------------------------------------------------+
```

## Pose update

At some rate (decided by the front-end), the front-end should collect
pose data and send to the back-end.
The update should be timestamped and the back-end will include the same
timestamp in the repose.

### front -> back

```text
0       1       2       3       4       5       6       7
+-------------------------------+-------------------------------+
|         Timestamp (32)        |             x (32)            |
+-------------------------------+-------------------------------+
|             y (32)            |             z (32)            |
+-------------------------------+-------------------------------+
|            yaw (32)           |           pitch (32)          |
+-------------------------------+-------------------------------+
|            row (32)           |
+-------------------------------+
```

## Trace results

At some rate (decided by the back-end), the back-end should send ray-traced
image to the front-end.
The result should include a timestamp representing which pose data this
rendered image is using.

### back -> front

```text
0       1       2       3       4       5       6       7
+-------------------------------+-----------------------+-------+
|         Timestamp (32)        |  Color of ray 1 (24)  | Color |
+---------------+---------------+-------+---------------+-------+
| of ray 2(24)  |  Color of ray 3 (24)  |  Color of ray 4 (24)  |
+---------------+---------------+-------+-----------------------+
|                       Colors continued ...                    |
+---------------------------------------------------------------+
```
