Packet
[![Website](https://img.shields.io/website-up-down-green-red/http/shields.io.svg?label=my-website)](https://sites.google.com/view/rodrigoholztrattner)
[![Linkedin](https://img.shields.io/badge/linkedin-updated-blue.svg)](https://www.linkedin.com/in/rodrigoholztrattner/)
=====


Packet is a C++ virtual file system library built primary for games, it allows any application to generate and use a *bundle* to store and access resources efficiently. The main characteristics are:

 * Simple, every important functionality is located in 3 classes only.
 * Fast, it has 2 ways of interaction, the first one allows the user to view and edit the *bundle* as a virtual system (create directories, files, delete, move, etc). the second one operates as a read-only mode but it is optimized for fast file fetching.
 * Customizable, all the loading features can be modified, you can use your favourite memory allocation system or threaded job loaders.

# How It Works

- You will start creating a main packet object (and its respective file).
- The packet object has an internal iterator object, you are going to use it for any virtual-system related methods like `MakeDir`, `Seek`, `Put`, `Get`, `Delete`, etc.
- As files are being added, the packet file will create fragments, those fragments are used to distribute the internal resource data into several pieces, so you will never have a huge one-only file with 20GB or more.
- When there is no need for modifying the virtual system structure anymore, all resources can be accessed using a read-only way that is optmized for speed (compile-time hashing and less map lookups) that accepts the filepath as input, like *"/images/test.png"*. 
- There is an `Optimize` method that will (hopefully) reduces the total of fragmented data inside the fragments.

> It's important to notice that the **only thread-safe** method is the load one from the PacketFile class, using multiple threads for every other method will cause undefined behaviour.
> Take a look at the Fast File Loading section for more details.

# Dependencies

* The current only depenency is the [JSON for Modern C++](https://github.com/nlohmann/json) library as we store some of the headers using readable JSON text files.

# Install

Just copy & paste every *.h* and *.cpp* from the root folder into your project. To use the library just include the **Packet.h** as it includes
every other file needed.

The project was built using the Visual Studio 2017 and should work properly just by opening its solution file. The main file located here is an example application that simulates the virtual file system with console commands.

The entire solution was made using pure C++ so if you want you can just copy-paste the files and use them in your project. Just remember the only dependency: [JSON for Modern C++](https://github.com/nlohmann/json).

# Getting Started

Let's start by creating our packet object. First we will need to provide the name and the maximum fragment size we are going to use.
For this guide/examples the packet name will be **"wonderland"** and **67108864** (64mb) will be used as our maximum fragment size.

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
