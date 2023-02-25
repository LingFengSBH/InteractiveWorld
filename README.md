# InteractiveWorld
 Source code of UE marketplace plugin "Interactive World"

## About this plugin
Developed for: Unreal Engine

Supported Engine Versions: UE5.0, UE5.1

Development Platforms: (Win64)

## Description
Easily create interactive snow, mud, water and foliage area in large world. 

Simulate with RenderTarget, and following player to support in large world.

You can use Interact Volume to designate areas in which specific DraingBoard should simulate.

Auto active and shut off  or sleep simulating.

## Documentation

Interactive World Documentation (English):
> https://docs.google.com/document/d/1vmjYRa4ZkMndAtadX8vMQbPki_YEK0Qs/edit?usp=share_link&ouid=112371780473520018513&rtpof=true&sd=true

Interactive World Documentation (简体中文):
> https://docs.google.com/document/d/1nwaMYKUeB_QoZBh7jro3i8RbF9blP_uA/edit?usp=share_link&ouid=112371780473520018513&rtpof=true&sd=true

# INSTALL
## Install in UE Marketplace (recommend)
If you just want to use this plugin, you can find and download it in UE Marketplace:
> https://www.unrealengine.com/marketplace/en-US/product/interactive-world

## Clone Source Code in github
### Who need this
If you want to modify the code by your self.

If you want to get newest update of this plugin.

If you have some issues want to report.

If you want to contribute to this plugin.

### Before Clone
To avoid conflict with plugin in engine folder

You'd better Uninstall "Interactive World" plugin in Epic Luncher if you have installed it in marketplace

### Install
Create a folder named "plugins" in your project folder (if you have created plugin in this project, there would exist one).

Clone this plugin in your "plugins" folder.

Open your project and build plugin.

# TO DO LIST
## System
- [ ] Sequencer Support
  - [X] Allow DrawingBoard follow specific actor 
  - [X] Manually let Interact Brush enter or leave Interact Volume
  - [ ] Example Map
  - [ ] Documentation update
  
 - [ ] Scalability Documentation
   - [ ] Create your own Drawing Board
   - [ ] Create your own Interact Brush
 
## Drawing Board
- [ ] Snow
  - [ ] Add a physics component for snow
  - [ ] Recreate Snow Material

- [ ] Water
  - [X] Add some new interact moode to adapt ocean surface (instead of line trace)
  - [ ] Add swimming system
  - [ ] Water cuastic
  - [ ] New Water Material
 
 - [ ] Foliage
   - [ ] Direaction of displacement or something
   - [ ] New foliage mesh
  
 - [ ] Mud
   - [ ] Parrallax mud
   
## New Functions
- [ ] Interactive Fog
- [ ] N-S Equation Fluid

# Change Log
## Version 1.0
### 2023.2.11
Published UE Marketplace.

## Version 1.1
### 2023.2.13
Add Low Quality options in BP_DrawingBoard_Snow.

Add TargetActor in BP_DrawingBoard,allow customize following actor.Which is useful when using with Sequencer.

Fix Bug:Wheel brush draw mirror-flipped pattern wehen move backward.

Export FreshSnowFallTime parameter in material to BP and Editor Utilities Widget.

Fix Bug:Self Shadow incorrect.

Update 1.1 version in UE Marketplace.

## Version 1.2
### 2023.2.14
Add Manual Interact Brush Enter/Leave Interact Volume function.For InteractBrushes that do not attached to a primitive component with collision.

### 2023.2.15
Bug fix: Brush doesn't shut off correctly after leaving.

### 2023.2.16
Added DeBug tool, which can draw debug shapes for InteractBrushes and DrawingBoards.

### 2023.2.17
FixBug: Wheel draw incorrectly when moving along rotation axis 

### 2023.2.21
FixBug: Fix wheel draw incorrectly when rotation > 180

### 2023.2.25
Update Interact brush to adapt it to more scenes

Add sequencer demo and ocean demo
