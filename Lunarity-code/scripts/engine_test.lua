function transform_update()
   if keyboard["scancode_y"] then
   transform_x = transform_x + 1
   print("x is now", transform_x)
   end
   return transform_x, transform_y
end
function renderable_update()
   renderable_w = renderable_w + 1
   return renderable_w, renderable_h
end
function walking_component()
   return 1
end
function running_component()
   return 1
end
function setup()
   print("awooga")
end
