function transform_update()
   transform_x = transform_x + 1
   return renderable_w, renderable_h
end
function renderable_update()
   renderable_w = renderable_w + 1
   print(renderable_h)
   return renderable_w, renderable_h
end
function walking_component()
   print(walking_component_playing)
   return 1
end
function running_component()
   print(running_component_playing)
   return 1
end
