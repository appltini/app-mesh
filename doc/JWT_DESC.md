# JWT authentication  ![jwt-logo](https://jwt.io/img/pic_logo.svg)
[JSON Web Tokens](https://jwt.io/)

------

JSON Web Tokens are an open, industry standard RFC 7519 method for representing claims securely between two parties.

![jwt_auth_process](https://cdn2.auth0.com/docs/media/articles/api-auth/client-credentials-grant.png)

What is supported:

> * REST login use JWT standard
> * Provide login and auth API to run as a stand-alone JWT server
> * Support centralized user & role DB server by Consul

What is **not** supported:
> * Redirect authentication to another JWT server is not supported
> * LDAP is not supported


### GET JWT token

Method | URI | Body/Headers | Desc
---|---|---|---
POST| /appmesh/login | UserName=base64(uname) <br> Password=base64(passwd) <br> Optional: <br> ExpireSeconds=600 | JWT authenticate login, the max ExpireSeconds is 24h

```shell
curl -X POST -k -s -H "UserName:$(echo -n user | base64)" -H "Password:$(echo -n Password | base64)" -H "ExpireSeconds:2" https://localhost:6060/appmesh/login | python -m json.tool
```
The REST will response bellow json when authentication success:

```json
{
    "AccessToken": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJleHAiOjE1ODg5Mjc1MTgsImlhdCI6MTU4ODkyNzUxNiwiaXNzIjoiYXBwbWdyLWF1dGgwIiwibmFtZSI6InVzZXIifQ.MRK0MH3hBw0ZbcIbSEtynFMkHSca2SYCCziX24VdT0w",
    "expire_time": 1588927516172435071,
    "profile": {
        "auth_time": 1588927516172426859,
        "name": "user"
    },
    "token_type": "Bearer"
}
```

| response   |  desc   | 
| --------   | -----  |
| AccessToken     | JWT token content |
| expire_time |  UTC time millisecond the token will expire, is the server time add the input ExpireSeconds| 
| auth_time | the server UTC time millisecond |
| token_type | JWT standard "Bearer" | 


### Use JWT token for REST request

Method | URI | Body/Headers | Desc
---|---|---|---
POST| /appmesh/auth | headers: <br> Authorization=Bearer token_str  <br> Optional: <br> AuthPermission=permission-id | JWT token authenticate

```shell
curl -s -X POST -k -H "Authorization:Bearer $jwt_token" -H "AuthPermission:app-view"  https://127.0.0.1:6060/appmesh/auth | python -m json.tool
```
The REST will response bellow json when authentication success:
```json
{
    "permission": "app-view",
    "success": true,
    "user": "user"
}
```

