# Path design

Calculate the value for a path

- Each path begins with  a `.` which signifies a JSON object.
- The path `.name` represents a path that has a depth of 1 and no equality operator. This would match the JSON `{"name":"Barry"}`
- The path `.job.title = "Cyber Engineer"` represents a path with a depth of 2 and an equality expression for the value of `title`. This would match the JSON `{"job" : {"title":"Cybber Engineer"}}`.
- The path `.a.b[1].c` represents a path that references the value in an array. It would match `{"a": {"b":[{"b":"B"},{"c":"C"}]}}` and would return `"C"`.
- The path `.a.b[].c` represents a path that references some value in an array without reference to an index. It It would match `{"a": {"b":[{"b":"B"},{"c":"C"}]}}` and would return `"C"` for the value `c` and `null` for any other entries in the array. For the JSON `{"a": {"b":[{"c":"B"},{"c":"C"}]}}` it would return `"B"` and `"C"` as the key `c` appears twice in the array.