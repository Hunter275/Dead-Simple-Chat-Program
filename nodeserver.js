// https://nodejs.org/en/docs/guides/anatomy-of-an-http-transaction/
var http = require('http');

http.createServer(function(request, response) {
  var headers = request.headers;
  var method = request.method;
  var url = request.url;
  var body = [];
  request.on('error', function(err) {
    console.error(err);
  }).on('data', function(chunk) {
    body.push(chunk);
  }).on('end', function() {
    body = Buffer.concat(body).toString();
    response.on('error', function(err) {
      console.error(err);
    });

    response.statusCode = 200;
    response.setHeader('Content-Type', 'text/plain');
    var responseBody = {
      headers: headers,
      method: method,
      url: url,
      body: body
    };

    response.write(Date.now().toString());
    response.end();
  });
}).listen(8080);
