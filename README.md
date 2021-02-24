 # __midge__

A graphical development framework and editor environment intended for innovative development of interactive visual applications.

#### Focus on visualization of data and errors
Allows quick evaluation and fixing of problems and iterative progression.

#### Automation of process and workflow generation

Using historical tracking of utilized actions and processes for statistical
suggestion for collaboration with the user to construct workflow processes. These
processes can be used for guidance and prediction of future user behaviour through
repeatable tasks and workflows. This reduces mechanical and cognitive expense to the
developer to increase productivity.

#### Dynamic recompilation

Utilizing [TinyCC](https://bellard.org/tcc/tcc-doc.html) to enable dynamic
interpretation and quicker readjustments of projects.

#### Maximal configuration of development environment

The development environment itself (as much as possible) is run inside the same 
process used to develop projects. Utilizing the dynamic recompilation allows
reconfiguration by the developer however it may serve best. This allows editing
of the development environment by using the development environment.

## Progress

_This project is still very much in the discovery process and no useful releases are
in any immediate future._

## Dependencies

>* [glslangValidator](https://github.com/KhronosGroup/glslang)
>* [TinyCC for midge](https://github.com/ShimmyShaman/tinycc)
>* [cglm](https://github.com/recp/cglm)
>* [vulkan sdk](https://www.lunarg.com/vulkan-sdk/)

## Installation

>* Have Ubuntu (or Linux in general, not tested).  
>* Ensure the Vulkan SDK has been installed.

```bash
      git clone --recurse-submodules https://github.com/ShimmyShaman/midge.git
```

>* Create a new folder named _glslang_ in the folder midge/dep/
>* Extract the contents of the appropriate package from [https://github.com/KhronosGroup/glslang/releases/tag/master-tot](https://github.com/KhronosGroup/glslang/releases/tag/master-tot) to midge/dep/glslang

```bash
      cd midge
      ./run.sh
```
