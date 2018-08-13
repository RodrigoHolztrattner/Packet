Packet
[![Website](https://img.shields.io/website-up-down-green-red/http/shields.io.svg?label=my-website)](https://sites.google.com/view/rodrigoholztrattner)
[![Linkedin](https://img.shields.io/badge/linkedin-updated-blue.svg)](https://www.linkedin.com/in/rodrigoholztrattner/)
=====


Packet is a C++ resource management library built primary for games. When developing it my focus was to achieve a high performatic system while at the same time allowing all the usual functionalities that a library like this should have. 

It uses two operation modes, a not-so-fast **edit** mode and the fast-and-optimized **condensed** mode, the first one allows the user to manager all of its resource files normally as if they were located physically on a "data" folder, the second one will compress those physical files into huge condensed ones, providing a much more faster access but removing any editing functionality.

I've being using this library internally for my projects but I decided to share it. Right now it has those characteristics:

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

### Creating the Packet System Object

There are 2 options when creating the main system object:

The first one should be used when the user will never attempt to request resource instances from different threads, if is guaranteed that it will only happen inside the same thread this should be your choice.

The second one should be used when the user will attempt to request resource instances from multiple threads, internally we will create *n* queues where *n* is equal to < total_number_of_threads > so each thread will 
use its own queue, also the user must provide a valid method (with this signature: *std::function<uint32_t()>*) to retrieve the current thread index (an index in range from 0 to n-1).

```c++
Packet::System* packetSystem = new Packet::System();
```
```c++
Packet::System* packetSystem = new Packet::System(<thread_index_retrival_method>, <total_number_of_threads>);
```

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

  MaterialInstance(Packet::Hash& _hash, Packet::ResourceManager* _resourceManager, Packet::ResourceFactory* _factoryPtr) :
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

### Creating Factories

Factories are a nicer way to provide much more flexibility when constructing 


### Creating the Packet Object

To create our packet object we will just allocate memory for it and initialize using the `InitializeEmpty` method:
```c++
Packet::Object* packetObject = new Packet::Object();
if (!packetObject->InitializeEmpty("Wonderland", 67108864))
{
	return false;
}
```

Now with the packet object we can start doing interesting things.

### Getting the Iterator

The iterator is responsible for all virtual file-system related methods, we will need to adquire a reference to it to continue:

```c++
auto packetIterator = packetObject->GetIterator();
```

### Creating Directories

To create directories you just need to use specify the new directory name or the path to the new directory.

```c++
packetIterator.MakeDir("resources");
packetIterator.MakeDir("resources/images");
```

### Seeking

Now we need a way to move inside the virtual file system, just use the `Seek` method for it.

```c++
packetIterator.Seek("resources");
packetIterator.Seek("images");
packetIterator.Seek(".."); // Back to resources
packetIterator.Seek(".."); // Back to root
packetIterator.Seek("resources/images");
```

### Listing

To get a list of all folders and files you can use the `List` method, it will return all folders and files from the current location (or from the given one).

```c++
packetIterator.List();
packetIterator.List("resources/images");
```

### Inserting Files and Data

You can insert any external file using the `Put` method, the first argument is always the *filepath* and the second (optional) is the *internal path* (with name) the file will be put.
This method can be used with a data ptr (you need to inform the size too)

```c++
packetIterator.Put("assets/textures/icon.jpg");
packetIterator.Put("assets/textures/icon.jpg", "resources/images/iconInput.jpg");
packetIterator.Put(validUnsignedCharData, dataSize);
packetIterator.Put(validUnsignedCharData, dataSize, "resources/images/iconInput.jpg");
```

### Getting Files and Data

To get a file from the virtual system, there is the `Get` method that receives the *path to the file* you want as the first argument and (optionally) the *output path* (including the name) where it will be located.
This same method can be used with a valid data ptr.

```c++
packetIterator.Get("resources/images/icon.jpg");
packetIterator.Get("resources/images/icon.jpg", "assets/icons/iconOutput.jpg"); 
packetIterator.Get("resources/images/icon.jpg", validUnsignedCharDataPtr); 
```

### Deleting Files and Folders

To delete a file or a folder, just use the `Delete` method.

```c++
packetIterator.Delete("resources");
packetIterator.Delete("resources/images");
packetIterator.Delete("resources/images/icon.jpg");
```

### Fast File Loading

We can use a `PacketFile` object to load file\data much faster then using the iterator methods.
This should only be used when the packet object is not being modified.

> When using the iterator, each string is parsed and hashed multiple times, causing a considerable overhead
> that could be noticeable for real time applications (games for example), the method that is described here
> uses **compiler-time hashing** and only do **two std::map lookups**.

To initialize the packet file and get a reference to it, we first need to provide our main packet object to
initialize a `PacketFileManager`. Using this manager we can request references to any file inside our
"bundle".

```c++
Packet::FileManager* fileManager = new Packet::FileManager(&packetObject);
```

Requesting a file is simple, we need to provide the file reference object, the file name and the load operating mode (**OnRequest**, **OnProcess** or **Assync**).
It's possible to determine if the memory allocation for the loading phase should be delayed until the real loading starts (more noticeable when the loading is assync), by default this is set to false.

*This method is thread-safe and allows multiple worker thread access.*

```c++
Packet::FileReference fileReference;
fileManager->RequestReference(&fileReference, "resources/images/icon.jpg", PacketFile::DispatchType::Assync);
```

> - OnRequest should be used when the file must be loaded on the current thread and inside the current request call.
> - OnProcess means that you need to call the **ProcessQueues** method first to start the loading process of all files inside the current load queue.
> - If Assync was your choice, the file will be loaded by a dedicated thread some time after the **ProcessQueues** method was called.
>
> It's possible to derive the PacketFile class to override the memory allocation and deallocation methods.

The file manager should be updated every frame (in real time situations) or at least when there is a new file inside the loading queue. For this
remember to use the **ProcessQueues** method.

```c++
fileManager->ProcessQueues();
```

When the file reference isn't needed anymore, you can release it using the **ReleaseReference** method:

*This method is thread-safe and allows multiple worker thread access.*

```c++
fileManager->ReleaseReference(&fileReference);
```

If you are using a workspace with multiple worker threads, there is the possibility to enable the multiple-queue mode, so, each thread
will have its own request queue and the synchronization will occur on the ProcessQueue method, for this just register the total number
of worker threads (the maximum number established) and the method to retrieve the thread index (inside the range from 0 to the total number
of worket threads).

```c++
// We will allocate 4 worker threads for the system, the other parameter is a lambda function
fileManager->UseThreadedQueue(4, []() 
{
    // Using my other library (Peon) for this example
    return Peon::GetCurrentWorkerIndex();
});
```

### Error Handling

If something wrong occurs you can retrieve an error object that contains the current operation status and any error code:

```c++
// Acquire the error object
auto errorObject = packetIterator.GetError();

// Print the current error code and message into the default output
errorObject.PrintInfo();
```

Alternatively you can get the error info and handle it by yourself. 

```c++
uint32_t errorCode;         
std::string errorString;

// Acquire the error info
errorObject.GetInfo(errorCode, errorString);
```

### Other

There is an `Optimize` method that will try to defragment our packet fragment files.
This method can take a while to conclude depending on the number of files your packet bundle has.
```c++
packetIterator.Optimize();
```

You can retrieve the current internal path (after using `Seek`) using the `GetCurrentPath` method like this:

```c++
std::string currentPath = packetIterator.GetCurrentPath();
```

-----
-----
-----

### Oops, a wild dinosaur appeared and stole the rest of this introduction, we will try to update this as soon as possible.
