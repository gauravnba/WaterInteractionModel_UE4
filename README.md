# WaterInteractionModel_UE4

![alt tag](Screens/WaterInteractionModelScreen.png)

A physics interaction model of a boat in water using nVidia PhysX, Waveworks and Unreal Engine. Since the UE4 Editor requires Waveworks to be integrated, I used a custom build of Unreal Engine, that can be found [here](https://github.com/NvPhysX/UnrealEngine/tree/WaveWorks)
This physics model is based on the Gamsutra article by Jaques Kerner, about a [Water interaction model for boats in video games](https://www.gamasutra.com/view/news/237528/Water_interaction_model_for_boats_in_video_games.php).
Ideally, the model should work for a boat of any shape in a body of water with any type of behaviour. However, since the model is still in development, it works for the boat model used.

## Steps to run simulation:
- Download and build the UE4 Waveworks build (4.15).
- In the project directory, right click on WaveworksTester.uproject.
- Select 'Switch Unreal Engine version...'
- In the dropdown menu, select the source where the UE4 Waveworks build is present.
- Once the project is on the correct UE4 build, you can play the simulation in the Editor.

## About the project

![alt tag](Screens/WaterInteractionLatest.gif)

The model is designed as a component that can be attached to any Actor, such that that Actor is now able to use the Water Interaction physics.
There are two main C++ classes that include the code required to run this interaction -
 - WaterPhysicsComponent - This includes all the logic required to run the physics simulation.
 - BoatPhysicsUtil - This includes the mathematic formulae used by the simulation.
 For more detailed information on the project, please read the dev diary at https://gnandagames.wordpress.com/blog/
 
 ### Issues currently working on:
  - Packaging the project to an exe file is not working.
  - Partially submerged triangles are not included in calcualation.
  - Balancing of the numbers to better suit boats of any size.
  - Addition of Slamming forces.
