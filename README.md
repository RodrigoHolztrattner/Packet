Packet
[![Website](https://img.shields.io/website-up-down-green-red/http/shields.io.svg?label=my-website)](https://sites.google.com/view/rodrigoholztrattner)
[![Linkedin](https://img.shields.io/badge/linkedin-updated-blue.svg)](https://www.linkedin.com/in/rodrigoholztrattner/)
=====


Packet is a C++ resource management library built primary for games. When developing it my focus was to achieve a high performatic system while at the same time allowing all the usual functionalities that a library like this should have. 

It uses two operation modes, a not-so-fast **edit** mode and the fast-and-optimized **condensed** mode, the first one allows the user to manager all of its resource files normally as if they were located physically on a "data" folder, the second one will use a merged/compressed version of those physical files, providing a much more faster access but removing any editing functionality.

I've being using this library internally for my projects but I decided to share it. I have a few additions planned to the future, right now it has those characteristics:

 * Two operation modes, **edit** (not really fast) and **condensed** (fast and furious)
 * Asynchronous loading (with some guaranted synchronous methods if the user needs synchronization)
 * Hot-reload when on **edit** mode
 * Creation of bundle-like files that are used on the **condensed** mode
 * Allows the user to manage how the resource and data memory allocation/deallocation will happen
 * Reference counting lifetime for resources
 * Resource dependency management (intra-resource dependency checks and corrections, explained below)

# Dependencies

Those are all the external libraries used here, they are located on the ThirdParty folder so including them shouldn't be a problem.

 - [JSON for Modern C++](https://github.com/nlohmann/json) used when creating resource-dependency files.
 - [Compile Time Type Information for C++](https://github.com/Manu343726/ctti) to deal with factory classes in a nicer way.
 - [simplefilewatcher](https://code.google.com/archive/p/simplefilewatcher/) so we can detect file changes on the fly.
 - [A single-producer, single-consumer lock-free queue for C++](https://github.com/cameron314/readerwriterqueue) the library name says everything.

# Install

The library itself is a Visual Studio 2017 project, but it should be compatible with any common C++ compiler although I have only tested it on Windows.

# Definitions

### Operation Modes

- The **edit** mode will allow you to access almost all functionalities without focusing alot on performance and optimizations, using it will enable you to manage your resource files as you would normally do (using
raw files directly on the current resource folder, the library won't be using the compressed data used on *condensed* mode), also you will be able to use the hot-reload feature (editing a resource at runtime can be 
handled, creating a new resource and swapping both pointers when everything is ready), In addition to that the library itself will perform some sanity checks to ensure there are no invalid resources trying to be 
used or if any operation would fail. In this mode the user can generate the compressed data user for the *condensed* mode (I will touch this further below).
- The **condensed** mode will operate only on compressed data, this mode will disable alot of features and checks, its use is focused when shipping you application (or building it on release mode) so there are no needs
to edit resources on-the-fly or perform validation checks, using this will be much more fast then its conterpart mode because .... hash bla bla

### Library Classes

- A **Resource** is an object that has its lifetime managed by the total number of instances and indirect references to it (also a resource can be marked to be permanent), it provides creation, loading, unloading 
and destruction methods to deal with the resource data, only one resource of each type can exist at the same time.
- A **ResourceReferencePtr<>** retains a temporary reference to a resource that will prevent it from being deleted even if all of its intances are released, the ideia is that this should be used when the user needs 
access to this resource in the future but it can't ensure that its instances won't be release until there. This object don't need to be release from the same threads that requested it so this is really usefull when 
using a different thread for rendering purposes or constructing a command queue that will be processed by the render thread, destructing this reference later.
- An **ResourceInstance** works like a reference to a resource (they are unique to each request) but they can also own other instances and depend on their initialization to be fully initialized. When requesting a 
resource instance the user has the option to pass a *ResourceBuildInfo* structure that will be explained further below. When the user needs to request an instance it should use a specialized pointer class called 
*ResourceInstancePtr*.
- An **ResourceInstancePtr<>** class works like a std::unique_ptr that points to a resource instance, it should be used in conjunction with move semantics, to check if it is valid the user can just do a simple if(pointer) ..., 
also it will auto release its instance object if it goes out of scope.
- A **ResourceFactory** is an interface class that is responsable for managing the creation and destruction of resources, instances and the resource data itself, each type of resource should have a factory associated with it.
- A **Hash** is an object that holds info about a resource path, also it has an internal identifier (the hash value itself) that is used to perform fast map-find operations when running on *condensed* mode.
- A **ResourceData** is an object that holds the resource data and expose its allocation and deallocation methods, so the user can use it as a base class and implement a new one using different allocation rules, by 
default it uses the standard allocation/deallocation methods (new and delete).

# General Rules

 * Always use the *ResourceReferencePtr* and *ResourceInstancePtr* to hold instances and references instead using their direct raw pointers, those especialized pointers should be used in conjunction with move semantics.
 * The user **can't** request *resource instances* or *resource references* when the packet system is being updated, there are some exceptions to this that I will explain further below but the user must ensure that the update 
phase will taken place alone, on its holy moment, without interruptions.
 * A *resource reference pointer* should be used when the user want's to ensure that a *resource* will be valid until this pointer is destructed, try to always use it when using multiple threads to ensure no race conditions 
will ever exist.

# Getting Started

All important classes, definitions, enums, etc are included on the *Packet.h* file, so when the user wants to use this library this is the file that should be included.

Currently I'm using this library intensely on my [Vulkan rendering engine](https://github.com/RodrigoHolztrattner/WonderlandProject/tree/master/WonderlandProject/Engine/Resource) if you need some guidance.

### Creating and Initializing the Packet System Object

The packet system class has 2 constructors: The first one should be used when the user will never attempt to request resource instances from different threads, 
if is guaranteed that it will only happen inside the same thread this should be your choice.

The second one should be used when the user will attempt to request resource instances from multiple threads, internally we will create *n* queues where *n* is equal to < total_number_of_threads > so each thread will 
use its own queue, also the user must provide a valid method (with this signature: *std::function<uint32_t()>*) to retrieve the current thread index (an index in range from 0 to n-1).

```c++
Packet::System* packetSystem = new Packet::System();
```
```c++
Packet::System* packetSystem = new Packet::System(<thread_index_retrival_method>, <total_number_of_threads>);
```

Now to initialize it you have to indicate what mode it will operate (*edit* or *condensed*) and what is the main resource path for your application (all resource paths will be relative
to this path). For example:

```c++
if (!packetSystem->Initialize(Packet::OperationMode::Edit, "Data"))
{
    // Handle the problem
    // ...
}
```

### Creating the Condensed File

To operate using the *condensed* mode you first need to generate the condensed files, this can be achieved by calling the *ConstructPacket()* method like this:

```c++
if(!packetSystem->ConstructPacket())
{
    // Handle the problem
    // ...
}
```

This method will take each file inside the data folder (the initialization path) and join them together (ignoring extensions used internally) in multiple condensed files.
In the future I plan to support merging two condensed packs of files together so things like updating a game files (because of a new patch) will be possible.

### Updating the Packet System

There is an update method that must be called on your logic frame (update frame) so the packet system can do its job, when this update is running you must **ensure** no
resource request will be done (except requests that are done by a resource instance being constructed).

```c++
packetSystem->Update();
```

### Checking if a Resource Exist

To check if a resource exist you must call the *FileExist()* method:

```c++
bool fileExist = packetSystem->FileExist("resource-path.extension");
```

### Registering a Factory Class

For each type of resource that you want to use you must register a corresponding factory class, the class creation will be addressed further on. This factory class
will be used when creating and destructing objects relationed with the given resource. For now you must use your own class (inheriting it from my interface) and it 
should be passed as an unique_ptr:

```c++
packetSystem->RegisterResourceFactory<MyResource>(std::make_unique<MyFactory>());
```

### Requesting a Resource Instance

Requesting a resource instance is really simple, just call the *RequestResource()* and it will process your request on the next *Update()* call, here you must use
a *Packet::ResourceInstancePtr* object that will hold a reference to the instance object:

```c++
Packet::ResourceInstancePtr<MyResource> myResourceInstance;

bool requestResult = packetSystem->RequestResource<MyResource>(myResourceInstance, "resource-path.extension");
```

Optionally here you can pass a *ResourceBuildInfo* structure with more detailed info about how to load and handle this resource request, I explain about each possible
option further below.

### Requesting a Resource Reference

To request a resource reference you must call the *GetResourceReference<>()* method from a valid resource instance object, this will create, register and return a valid
*Packet::ResourceReferencePtr* that should be used to hold the reference itself:

```c++
Packet::ResourceReferencePtr<MyResource> myResourceReference;
myResourceReference = myResourceInstance->GetResourceReference<MyResource>();
```

Remember that this reference must not be used instead of a instance one, a reference is aimed to be used when the resource access must be done by a separated thread and
during this time you can ensure that the initial instance will exist, a reference will prevent the resource from being deallocated.

# Resource Related Classes

Here I will give some examples on how to create and structure all classes that you need to create to integrate this library into your application, you can check [my resource/instance/factory implementations in my other project](https://github.com/RodrigoHolztrattner/WonderlandProject/tree/master/WonderlandProject/Engine/Resource)
instead reading all the future sections.

### Structuring a Resource Class

Here I will give an example of how to create your resource class, there are some methods that you must override but otherwise you are free to do anything you want

The resource constructor is always called when inside the update phase so calling it is considered synchronous but its destruction is by default asynchronous, this can be changed when requesting
a *resource instance* for this resource by passing a the build info parameter with different arguments.

Those are the virtual methods that you should override:

* When a resource is being loaded this method will be called asynchronous (by an internal loading thread).
```c++
virtual bool OnLoad(Packet::ResourceData& _data, uint32_t _buildFlags, uint32_t _flags) = 0;
```

---

* After the resource was loaded, on the next update phase for the packet system this method will be called, here the user can synchronize the already processed resource data with the
application, if no synchronization is needed just return true here.
```c++
virtual bool OnSynchronization() = 0;
```

---

* Right after the resource was marked to deletion (when inside the update phase for the packet system) this method will be called (synchronous) so the user can perform any pre-deletion
action that needs to be synchronized. Again this can be ignored if no synchronization is needed.
```c++
virtual bool OnDesynchronization() = 0;
```

---

* Right before a resource destruction occur, this method will be called so the user can perform any action with the current data, it will remain valid until this method returns.
```c++
virtual bool OnDelete(Packet::ResourceData& _data) = 0;
```

---

* This method is only called when on Edit mode and **if** the *createResourceIfInexistent* build info parameter was set to true, so if the user tried to load a resource that doesn't 
exist the load thread will call this method (asynchronous), here the user is supposed to create and manage its own resource data.
```c++
virtual bool OnCreation() { return true; }
```

A simple example can be seen below:

```c++
class MyResource : public Packet::Resource
{
protected:

  // The OnLoad() method (asynchronous method)
  bool OnLoad(Packet::ResourceData& _data, uint32_t _buildFlags, uint32_t _flags) override
  {
    // Use the methods _data.GetData() and _data.GetSize() to use the resource data here, don't forget the flags
    m_ResourceData.Initialize(_data.GetData(), _data.GetSize());

    return true;
  }

  // The OnDelete() method (asynchronous method)
  bool OnDelete(Packet::ResourceData& _data) override
  {
    // Shutdown any internal data, here the Packet::ResourceData can still be used
    m_ResourceData.Shutdown();

    return true;
  }

  // The OnSynchronization() method (synchronous method when calling the update() method)
  bool OnSynchronization() override
  {
    // This method is synchronized with the update phase for the packet system
    Globals::GPU::SynchronizeResource(this, &m_ResourceData);

    return true;
  }

  // The OnDesynchronization() method (synchronous method when calling the update() method)
  bool OnDesynchronization() override
  {
    // This methos is synchronized with the update phase too
    Globals::GPU::DesynchronizeResource(this);

    return true;
  }

private:

  // Material shader hashes
  MyResourceData m_ResourceData;
};
```

### Structuring a Resource Instance Class

A resource instance works like a reference to a resource object but at the same time each instance can have its own data, this is really usefull when for example, 
loading a skeleton resource (for animation) that contains info about each bone and also a GPU vertex buffer, so each instance can have its own bone transform matrix
that is maintained and update separated from the resource object itself.

The instance constructor is called right before its request (by the calling thread), its destructor is synchronized with the update phase for the packet system.
An instance can own other instances, in this case the user can register a dependency between those instances, only when all dependent instances are fully constructed
the method *OnDependenciesFulfilled()* (expalined below) will be called.

Those are the virtual methods that the user must override:

* This method is called when inside the update phase by default, the user can allows it to be asynchronous by setting true on the *asyncInstanceConstruct* build 
info parameter when requesting the instance.
```c++
virtual void OnConstruct() = 0;
```

---

* This method is called whan all instances that this one depends on are constructed, this is called when inside the update phase. If there are no dependencies it 
will be called right after the *OnConstruct()* method.
```c++
virtual void OnDependenciesFulfilled() = 0;
```

---

* This method will be only called when on Edit mode and if the resource that this instance reference was reconstructed, at this state this instance already points
to the new resource so its data must be reconstructed using it.
```c++
virtual void OnReset() = 0;
```

A simple example can be seen below:

```c++
class MyInstance : public Packet::ResourceInstance
{
public:

  MyInstance(Packet::Hash& _hash, Packet::ResourceManager* _resourceManager, Packet::ResourceFactory* _factoryPtr) :
    Packet::ResourceInstance(_hash, _resourceManager, _factoryPtr)
  {
  }

protected:

  void OnConstruct() override
  {
    // Here the resource data can be accessed using the GetResource() method
    m_InstanceData.Build(static_cast<MyResource*>(GetResource()));
  }

  void OnDependenciesFulfilled() override
  {
    // Called when all instances that we depends on are constructed
    // ...  
  }

  void OnReset() override
  {
    // Reconstruct this instance data, here the new resource is already valid (just use GetResource())
    m_InstanceData.Reset(static_cast<MyResource*>(GetResource()));
  }

private:

  MyInstanceData m_InstanceData;
};
```

### Structuring a Factory Class

Factories are a nicer way to provide much more flexibility when constructing resource related objects (resources, resource instances and resource data), the user can use
them to provide custom memory allocation features or insert custom data when allocating them.

Those are the virtual methods that the user must override:

* Called right after a new instance is requested (from the packet system). The user must return a valid unique_ptr for a new instance.
```c++
virtual std::unique_ptr<Packet::ResourceInstance> RequestInstance(Packet::Hash _hash, Packet::ResourceManager* _resourceManager) = 0;
```

---

* When a instance is being released, called only when inside the update phase for the packet system. The user must release the unique_ptr object.
```c++
virtual void ReleaseInstance(std::unique_ptr<Packet::ResourceInstance>& _instance) = 0;
```

---

* When a new resource object must be created, called only when inside the update phase for the packet system. The user must return a valid unique_ptr for a new resource object.
```c++
virtual std::unique_ptr<Packet::Resource> RequestObject() = 0;
```

---

* When a new resource object must be released, called only when inside the update phase for the packet system. The user must release the unique_ptr object.
```c++
virtual void ReleaseObject(std::unique_ptr<Packet::Resource>& _object) = 0;
```

---

* When a resource data must be allocated, called asynchronous by the loading thread. The user must allocate the data for the *Packet::ResourceData* input parameter.
```c++
virtual bool AllocateData(Packet::ResourceData& _resourceDataRef, uint64_t _total) = 0;
```

---

* When a resource data must be deallocated, called asynchronous by the deletion thread. The user must deallocate the data for the *Packet::ResourceData* input parameter.
```c++
virtual void DeallocateData(Packet::ResourceData& _data) = 0;
```

A simple example can be seen below:

```c++
class MyFactory : public Packet::ResourceFactory
{
public:

  // Request a new instance
  std::unique_ptr<Packet::ResourceInstance> RequestInstance(Packet::Hash _hash, Packet::ResourceManager* _resourceManager) override
  {
	return std::unique_ptr<Packet::ResourceInstance>(new MyInstance(_hash, _resourceManager, this));
  }

  // Release a instance
  void ReleaseInstance(std::unique_ptr<Packet::ResourceInstance>& _instance) override
  {
	_instance.reset();
  }

  // Request a new object
  std::unique_ptr<Packet::Resource> RequestObject() override
  {
	return std::unique_ptr<Packet::Resource>(new MyResource());
  }

  // Release an object
  void ReleaseObject(std::unique_ptr<Packet::Resource>& _object) override
  {
	_object.reset();
  }

  // Allocates the given amount of data for the resource creation
  bool AllocateData(Packet::ResourceData& _resourceDataRef, uint64_t _total) override
  {
	// By default this use the standard allocator
	_data.AllocateMemory(_total);

	return true;
  }

  // Deallocates the given data from the resource
  void DeallocateData(Packet::ResourceData& _data) override
  {
	// By default this use the standard allocator
	_data.DeallocateMemory();
  }
};
```

### Using Different Resource Build Info Parameters

There is a build info object that can be passed when requesting a new *resource instance*, this object can influence in many aspects for this request, its parameters are:

**Build Flags** are passed to the resource *OnLoad()* method, **two same resources with different build flags are considered different from each other by the system**, 
requesting the same resource (same *hash*) but with different build flags will result in multiple resource creations, this can be usefull when you need to request a shader
for example and must compile it using custom flags that influences directly the code (like using a "super fragment shader" and fetching those flags to determine wich 
rendering features must be used, then compiling the shader with the respective defines set).
```c++
uint32_t buildFlags = 0;
```

**Flags** are passed to the resource *OnLoad()* method, differently from the *build flags*, using different flags **does not** make resources with the same hash different 
from each other, they are like normal flags, you can use they for whatever you want.
```c++
uint32_t flags = 0;
```

**Async Instance Construct** (by default true) when set specifies that when requesting a new *resource instance*, if the *resource* already exist and is ready to be used this
new instance can have its *OnConstruct()* method called right after the request call (by the requesting thread), if this parameter is false this will only happens when inside 
the update phase for the packet system.
```c++
bool asyncInstanceConstruct = true;
```

**Async Resource Object Deletion** (by default true) sets if the *resource* object, when marked to deletion, can be released by its factory method *ReleaseObject()* asynchronous, 
in other words if this method can be called by the deletion thread or if it should be called only when doing the update phase for the packet system.
```c++
bool asyncResourceObjectDeletion = true;
```

**Create Resource if Inexistent** (by default false) sets if in the case there isn't a valid resource with the given hash, a resource object should be created anyway, this **only**
works when on *edit* mode and if that happen a call to the *OnCreation()* method will be made instead calling *OnLoad()*. This resource won't call its factory methods *AllocateData()* 
and *DeallocateData()*, the user is responsible for managing the data itself on this case. 
```c++
bool createResourceIfInexistent = false;
```

**Created Resource Should Load** (by default false) when true will enforce a call to the resource *OnLoad()* method when creating a new one (using the *createResourceIfInexistent* 
parameter) right after the *OnCreation()* call.
```c++
bool createdResourceShouldLoad = false;
```

**Created Resource Should Auto Save** (by default false) when true and when on *edit* mode will make a created resource be saved with its creation hash value when it goes out of scope
(released by having no instances or references).
```c++
bool createdResourceAutoSave = false;
```

---
---
---

### Oops, a wild dinosaur appeared and stole the rest of this introduction, I will try to update this as soon as possible.
