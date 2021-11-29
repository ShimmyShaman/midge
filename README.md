 # __midge__

A graphical development framework and editor environment intended for innovative development
of interactive visual applications.

### Purpose

An experimental project intended to investigate, explore and innovate the techniques and
workflows involved in future application development. Encroaching upon the era of
machine-assisted application development, this project intends to add to the future of IDE
and game-engine feature-sets to improve development speed and capabilities and, ultimately,
end-user experiences.

### Primary Feature-Set Pillars

#### Machine-Assisted Collaborative Development

Utilising advancements and techniques in machine learning and optimization algorithms to
produce tools and workflows that offer developers increased productivity. Providing developers
the ability to customize and specify process enhancements

#### Development and Production Automation

Utilising historical tracking and data science along with statistical methods of employed
actions and processes for suggestion and automation. Reducing repeatable tasks and workflows
and, consequently, mechanical and cognitive expense to the developer to increase productivity.

#### Dynamic recompilation

Utilizing [TinyCC](https://bellard.org/tcc/tcc-doc.html) and in-built semantic system to
enable better internal representation and selective recompilation of source code. The ability
to offer the power of dynamic interpretation of runtime-active project applications, whilst
seeking to avoid their thorny details, to allow quicker preview and increase iterative
productivity.

#### Maximal configuration of development environment

The development environment itself (as much as possible) is run inside the same process used
to develop projects. Utilizing the dynamic recompilation allows reconfiguration by the
developer however it may serve best. This allows editing of the development environment in the
process of using the development environment.

#### Monitoring and Visualization of data and errors

Allow quick evaluation and identification of problems and errors in any development project.

## Progress

_This project is actively in iterative prototype development._

## Contributions

Ideas, relevant links and concepts are most welcome.

Discussion available on [Discord](https://discord.gg/TSu23fw8ES)

## Dependencies

>* [glslangValidator](https://github.com/KhronosGroup/glslang)
>* [TinyCC for midge](https://github.com/ShimmyShaman/tinycc)
>* [cglm](https://github.com/recp/cglm)
>* [vulkan sdk](https://www.lunarg.com/vulkan-sdk/)

## Installation

> ### _Disclaimer_
> _This project is developed deeply and not broadly. I make it work on my computer
   so I can discover what can become of it. It won't probably won't work on a different setup
   (and sometimes doesn't work on my setup)._

>* Have Ubuntu with XCB/XLIB (has to be installed seperately with Ubuntu-20).  
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
