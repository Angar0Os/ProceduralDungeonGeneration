# ProceduralDungeonGeneration
This is a school project that aims to create procedural dungeon generation. It must use Delaunay triangulation on a version of Unreal Engine 5+. We had three weeks to complete this project.

## Draw Debug Colors

 - Blue --> Super triangle      | Starting triangle surrounding all rooms.
 - Yellow --> Valid Triangles   | Final triangles 
 - Red --> Bad Triangles        | Triangles whose circumcircle contains the current point 
 - Green --> Polygon Edges      | Edges forming the boundary after removing the bad triangles
 - Cyan --> CurrentPoint        | Actual Point inserted in triangulation 