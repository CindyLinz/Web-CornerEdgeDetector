Module =
  on-runtime-initialized: !->
    Module.detect = Module.cwrap \detect, void, ['number', 'number', 'number', 'number', 'number', 'number', 'number', 'number']
    Module.mono = Module.cwrap \mono, void, ['number', 'number', 'number']
    Module.composite = Module.cwrap \composite, void, ['number', 'number', 'number', 'number']

xhr = new XML-http-request
  ..onreadystatechange = !->
    if xhr.ready-state == 4
      xhr.onreadystatechange = new Function
      new Function(\Module, xhr.response-text)(Module)
  ..open \GET, \detector.js
  ..send void

canvas = document.query-selector \canvas

img-orig = new ImageData 10, 10
img-mono = new ImageData 10, 10
img-corner = new ImageData 10, 10
img-edge = new ImageData 10, 10

draw = !->
  console.log \draw!, img-orig.width, img-orig.height
  canvas.width = img-orig.width
  canvas.height = img-orig.height
  ctx = canvas.get-context \2d

  img = switch document.query-selector 'input[name=color]:checked' .value
    | \black =>
      ctx.fill-style = \#000
      ctx.fill-rect 0, 0, canvas.width, canvas.height
      ctx.get-image-data 0, 0, canvas.width, canvas.height
    | \white =>
      ctx.fill-style = \#fff
      ctx.fill-rect 0, 0, canvas.width, canvas.height
      ctx.get-image-data 0, 0, canvas.width, canvas.height
    | \mono =>
      img-mono
    | \color =>
      img-orig

  buf = Module._malloc img.data.length
  Module.HEAPU8.set img.data, buf

  if document.query-selector 'input[name=show_corner]:checked'
    buf2 = Module._malloc img-corner.data.length
    Module.HEAPU8.set img-corner.data, buf2
    Module.composite buf, buf2, img.width, img.height
    Module._free buf2
  if document.query-selector 'input[name=show_edge]:checked'
    buf3 = Module._malloc img-edge.data.length
    Module.HEAPU8.set img-edge.data, buf3
    Module.composite buf, buf3, img.width, img.height
    Module._free buf3

  ctx.put-image-data new ImageData(new Uint8ClampedArray(Module.HEAPU8.buffer, buf, img.data.length), img.width, img.height), 0, 0
  Module._free buf

document.query-selector 'input[type=file]' .add-event-listener \change, (ev)!->
  file = ev.target.files.0
  img = new Image
    ..src = URL.create-objectURL file
    ..onload = !~>
      w = img.natural-width
      h = img.natural-height
      canvas.width = w
      canvas.height = h

      ctx = canvas.get-context \2d
      ctx.draw-image img, 0, 0

      img-orig := ctx.get-image-data 0, 0, w, h

      buf = Module._malloc img-orig.data.length
      Module.HEAPU8.set img-orig.data, buf

      buf-corner = Module._malloc img-orig.data.length
      buf-edge = Module._malloc img-orig.data.length

      Module.detect buf, buf-corner, buf-edge, w, h, 1, 0.05, 1e-4

      img-mono := new ImageData new Uint8ClampedArray(new Uint8Array(Module.HEAPU8.buffer, buf, img-orig.data.length)), w, h
      img-corner := new ImageData new Uint8ClampedArray(new Uint8Array(Module.HEAPU8.buffer, buf-corner, img-orig.data.length)), w, h
      img-edge := new ImageData new Uint8ClampedArray(new Uint8Array(Module.HEAPU8.buffer, buf-edge, img-orig.data.length)), w, h

      Module._free buf
      Module._free buf-corner
      Module._free buf-edge

      draw!

document.query-selector \body .add-event-listener \change, (ev) !->
  switch ev.target.name
  | \color, \show_corner, \show_edge =>
    draw!
