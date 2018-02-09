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
- When there is no need for modifying the virtual system structure anymore, all resources can be accessed using a read-only way that is optmized for speed (compile-time hashing and less map lookups) that accepts the filepath as input, like *"\images\test.png"*. 
- There is an `optimize` method that will (hopefully) reduces the total of fragmented data inside the fragments.

# Install

The project was built using the Visual Studio 2017 and should work properly just by opening its solution file.
The entire solution was made using pure C++ so if you want you can just copy-paste the files and use them in your project. Just remember the only dependency: [JSON for Modern C++](https://github.com/nlohmann/json).

# Getting Started

Let's start by creating our packet object. First we will need to provide the name and the maximum fragment size we are going to use.
For this guide/examples the packet name will be *"wonderland"* and *67108864* (64mb) will be used as our maximum fragment size.

### Creating the Packet Object

To create our packet object we will just allocate memory for it and initialize using the `InitializeEmpty` method:
```c++
Packet::PacketObject* packetObject = new Packet::PacketObject();
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
packetIterator.MakeDir("resources\\images");
```

### Seeking

Now we need a way to move inside the virtual file system, just use the `Seek` method for it.

```c++
packetIterator.Seek("resources");
packetIterator.Seek("images");
packetIterator.Seek(".."); // Back to resources
packetIterator.Seek(".."); // Back to root
packetIterator.Seek("resources\\images");
```

### Listing

To get a list of all folders and files you can use the `List` method, it will return all folders and files from the current location (or from the given one).

```c++
packetIterator.List();
packetIterator.List("resources\\images");
```

### Inserting Files and Data

You can insert any external file using the `Put` method, the first argument is always the *filepath* and the second (optional) is the *internal path* (with name) the file will be put.
This method can be used with a data ptr (you need to inform the size too)

```c++
packetIterator.Put("assets\\textures\\icon.jpg");
packetIterator.Put("assets\\textures\\icon.jpg", "resources\\images\\iconInput.jpg");
packetIterator.Put(validUnsignedCharData, dataSize);
packetIterator.Put(validUnsignedCharData, dataSize, "resources\\images\\iconInput.jpg");
```

### Getting Files and Data

To get a file from the virtual system, there is the `Get` method that receives the *path to the file* you want as the first argument and (optionally) the *output path* (including the name) where it will be located.
This same method can be used with a valid data ptr.

```c++
packetIterator.Get("resources\\images\\icon.jpg");
packetIterator.Get("resources\\images\\icon.jpg", "assets\\icons\\iconOutput.jpg"); 
packetIterator.Get("resources\\images\\icon.jpg", validUnsignedCharDataPtr); 
```

### Deleting Files and Folders

To delete a file or a folder, just use the `Delete` method.

```c++
packetIterator.Delete("resources");
packetIterator.Delete("resources\\images");
packetIterator.Delete("resources\\images\\icon.jpg");
```

### Other

You can retrieve the current internal path (after using `Seek`) using the `GetCurrentPath` method like this:

```c++
std::string currentPath = packetIterator.GetCurrentPath();
```

If something wrong occurs you can retrieve an error object that contains the current operation status and any error code:

```c++
Packet::PacketError errorObject = packetIterator.GetError();
```


-----
-----
-----

### Oops, a wild dinosaur appeared and stole the rest of this introduction, we will try to update this as soon as possible.