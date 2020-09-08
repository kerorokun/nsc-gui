# Not-So-Convenient GUI C++ Framework
A minimal GUI framework with an emphasis on developer freedom. The NSC GUI is designed with the following goals:
1. It should be easily extensible. Developers are expected to extend this framework with their own elements, pipelines, etc.
2. It should be non-intrusive.
3. It should be performant.
4. It should have a small and clear code design.
5. It should be a framework not an environment.

## Provided features
This framework is still very much in its infancy so these features are bound to change. Additionally, since this is in many ways an experiment, alot of techniques used here are not necessarily industry-standards.
* Type erasured object containers.
* Concepts inspired by Sean Parent's [talk](https://www.youtube.com/watch?v=QGcVXgEVMJg). This allows developers to write classes that implement a specific interface and have the framework use that class without needing the class to inherit from a specific abstract class.
* Multi-channel Signed Distance Field fonts ([see here](https://github.com/Chlumsky/msdfgen)) with subpixel rendering. This is a very new type of font rendering. It allows for fonts to be rendered very sharply at all scales without needing to precompute the font at a specified font size. This is very experimental right now
* A generic slot and signals system for event handling. This is inspired heavily by Qt.
* A generic UI element that comes attached with slots that the user can add.
* Window creation and user inputs with GLFW. It also comes with a registry for slots and signals that the user can use to add signals easily.
* Rendering pipelines to render images and text.

## Modules
To be added
