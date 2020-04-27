module TodoId =
  Identity.Make({});

module Todo = {
  type t = {
    id: TodoId.t,
    title: string,
  };
};

module Todos = {
  module Item = {
    type t = TodoId.t;
    let eq = (x1, x2) => x1->TodoId.toInt == x2->TodoId.toInt;
  };

  module Container =
    Dnd.MakeSingletonContainer({});

  include Dnd.Make(Item, Container);
};

type state = {
  todosIndex: array(TodoId.t),
  todosMap: TodoId.Map.t(Todo.t),
};

type action =
  | ReorderTodos(Dnd.result(Todos.Item.t, Todos.Container.t));

let reducer = (state, action) =>
  switch (action) {
  | ReorderTodos(Some(SameContainer(id, placement))) => {
      ...state,
      todosIndex:
        state.todosIndex
        ->ArrayExt.reinsert(
            ~value=id,
            ~place=
              switch (placement) {
              | Before(id) => `Before(id)
              | Last => `Last
              },
          ),
    }
  | ReorderTodos(Some(NewContainer(_)))
  | ReorderTodos(None) => state
  };

[@react.component]
let make = (~n: int, ~axis: Dnd.Axis.t) => {
  let initialState =
    React.useMemo0(() =>
      {
        todosIndex: Array.range(1, n)->TodoId.array,
        todosMap:
          Array.range(1, n)
          ->Array.reduce(
              TodoId.Map.make(),
              (map, id) => {
                let id = id->TodoId.make;
                map->Map.set(
                  id,
                  Todo.{id, title: "Todo " ++ id->TodoId.toString},
                );
              },
            ),
      }
    );

  let (state, dispatch) = reducer->React.useReducer(initialState);

  <Todos.DndManager
    onDragStart={(~itemId as _itemId) =>
      BrowserLogger.infoWithData2(__MODULE__, "AppHook", ("Event", "DragStart"), ("ItemId", _itemId));
      //[%log.debug "AppHook"; ("Event", "DragStart"); ("ItemId", _itemId)]
    }
    onDropStart={(~itemId as _itemId) =>
    BrowserLogger.infoWithData2(__MODULE__, "AppHook", ("Event", "DragStart"), ("ItemId", _itemId));
      //[%log.debug "AppHook"; ("Event", "DropStart"); ("ItemId", _itemId)]
    }
    onDropEnd={(~itemId as _itemId) =>
    BrowserLogger.infoWithData2(__MODULE__, "AppHook", ("Event", "DragStart"), ("ItemId", _itemId));
      //[%log.debug "AppHook"; ("Event", "DropEnd"); ("ItemId", _itemId)]
    }
    onReorder={result => ReorderTodos(result)->dispatch}>
    <Todos.DroppableContainer
      id={Todos.Container.id()}
      axis
      className={(~draggingOver) =>
        Cn.make(["todos", "active"->Cn.ifTrue(draggingOver)])
      }>
      {state.todosIndex
       ->Array.mapWithIndex((index, id) => {
           let todo = state.todosMap->Map.getExn(id);

           <Todos.DraggableItem
             id={todo.id}
             key={todo.id->TodoId.toString}
             containerId={Todos.Container.id()}
             index
             className={(~dragging) =>
               Cn.make(["todo", "dragging"->Cn.ifTrue(dragging)])
             }>
             {`Children(todo.title->React.string)}
           </Todos.DraggableItem>;
         })
       ->React.array}
    </Todos.DroppableContainer>
  </Todos.DndManager>;
};
