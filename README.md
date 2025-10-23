# ProceduralDungeonGeneration
This is a school project that aims to create procedural dungeon generation. It must use Delaunay triangulation on a version of Unreal Engine 5.4.4. We had three weeks to complete this project.

## Draw Debug Colors

 - Blue --> Super triangle      | Starting triangle surrounding all rooms.
 - Yellow --> Valid Triangles   | Final triangles 
 - Red --> Bad Triangles        | Triangles whose circumcircle contains the current point 
 - Green --> Polygon Edges      | Edges forming the boundary after removing the bad triangles
 - Cyan --> CurrentPoint        | Actual Point inserted in triangulation 
	
## Project properties

 - See Spawn -> show spawning process of the rooms
 - Base piece count -> Starting piece counter before foing deluanay triangulation
 - Radius -> This is the size in which pieces caan spawn
 - Base point is the locaation where the radius base is located
 - Show MST (Minimum spanning tree) is a parameter that show MST process
 - Show super triangle is the parameter that activate debug lines on screen
 - Show delaunay is a parameter that shows the delaunay triangulation process
 - You can make it step by step and change step delay too
 - You can also decide to spawn corridors by using the parameter Generate Corridors

## Controls 

 - Press "E" to show corridors and final result of the dungeon.
