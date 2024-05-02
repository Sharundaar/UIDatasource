# Disclaimer

Be aware that this plugin and documentation is a work in progress. I've been reimplementing it from scratch and I'm simultaneously using it it in production, so things change as new issues and needs arises.

# Introduction

UIDatasource is a light MVVM plugin that aims to provide a performant, simple, and easy to integrate middle layer that helps with developping UI alongside an ever changing game.
At its core this plugin provides a simple way to implement UI that reacts to data change event, and to transfer data between the engine's backend and the UI in UMG.

# Principle

We're defining a Datasource as a node in a tree that we can declare with a path, for example "Player.Stats.Health" would define 3 datasources named "Player", "Stats", and "Health". Those would be link with Player being Stats parent, and Stats being Health parent.
We could also declare "Player.Name", which would define the "Name" datasoure with Player has Parent, and Stats as an immediate sibling.

```
Root
├ Player ┬ Stats ┬ Health: 100
│        │       └ Energy: 50
│        └ Name: Sharundaar
└ Group  ┬ ....
         ┆
```

We end up with a tree where each node can be seen as a structure that contains named member variables and other structures. We can then assign a node of the tree to a widget, and the widget will resolve its binding from the runtime shape of the structure, allowing graceful failure in case a binding fails.

# Datasource Archetypes

From the UI perspective we can define archetypes asset to declare what the expected shape of data we want. In this example we're defining the archetype of a datasource with 2 children, Tier and Icon, and we can specify the types of those fields.

![image](https://github.com/Sharundaar/UIDatasource/assets/6921222/9ac00ade-08f2-4970-8265-9038ab0df2e0)

We can then assign the archetype to our widget, and bind to any of the fields as necessary, in this case we've bound to Icon and Tier, and we're setting the UI when the bind succeed or the field change.

![image](https://github.com/Sharundaar/UIDatasource/assets/6921222/373f73b3-0e73-4bb2-b63f-6312195f6ada)

It's trivial to add a new field as it becomes necessary during development, or to refactor the archetype. The widget can also easily bind to a custom field not declared in the archetype for special treatment.

![image](https://github.com/Sharundaar/UIDatasource/assets/6921222/154fee11-8cd7-4bfb-80a7-1de01307b46a)

# Generating Datasource

There's a few ways to generate datasources, the simplest from blueprint is the Find Or Create Datasource node, and the Set Model Values node. Those two allows to create an empty datasource at a known path (Demo.InventoryItem in the example), and to generate a structure that conforms to an archetype (here configured to match the archetype created previously).
We can then assign the datasource to any user widget. It'll automatically register the datasource extension on the datasource if it didn't already have one.

![image](https://github.com/Sharundaar/UIDatasource/assets/6921222/9ef0cab3-2109-4299-9863-832e455845c7)

We can also generate the datasource given a root from C++ :

```cpp
UENUM(BlueprintType)
enum class EItemTier : uint8
{
	Basic,
	Uncommon,
	Rare,
	Legendary
};

void UBlueprintLibrary::GenerateItemDatasource(FUIDatasourceHandle Handle, TSoftObjectPtr<UTexture2D> ItemIcon) // Passing the Icon here, but in theory would come from whatever backend we have
{
	FUIDatasource& Datasource = Handle.Get_Ref();

	Datasource["Icon"].Set<FUIDatasourceImage>(ItemIcon);
	Datasource["Tier"].Set(static_cast<int32>(ItemTier::Legendary));
	Datasource["Quantity"].Set(5);
}
```

# Debugging

Finally the plugin comes with a debugger that allows us to visualize the datasource tree, resource statistics etc... In the future I plan on it showing history of datasource changes with frame data associated and more ! We can also easily modify the datasource tree directly from the debugger to see how widget reacts.

![image](https://github.com/Sharundaar/UIDatasource/assets/6921222/901b98b0-ae69-4ddb-b5e1-90f0c9309737)


