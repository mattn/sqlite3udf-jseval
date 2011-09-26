#include <stdlib.h>
#include <string.h>
#include <sqlite3ext.h>
#include <v8.h>

using namespace v8;

extern "C" {
  SQLITE_EXTENSION_INIT1
  static void js_eval_func(sqlite3_context *context, int argc, sqlite3_value **argv) {
    if (argc == 1) {
      HandleScope handle_scope;

      Persistent<Context> ctx = Context::New();
      Context::Scope context_scope(ctx);

      const char *js = (const char *)sqlite3_value_text(argv[0]);

      Handle<String> source = String::New((char*) js);
      Handle<Script> script = Script::Compile(source);

      TryCatch trycatch;
      Handle<Value> result = script->Run();

      if (result.IsEmpty()) {
        Handle<Value> exception = trycatch.Exception();
        String::Utf8Value error(exception);
        sqlite3_result_error(context, *error, 0);
      } else {
        String::Utf8Value utf8(result);
        char* p = strdup(*utf8);
        sqlite3_result_text(context, p, strlen(p), free);
      }
    }
  }
#ifdef _WIN32
  __declspec(dllexport)
#endif
  int sqlite3_extension_init(sqlite3 *db, char **errmsg, const sqlite3_api_routines *api) {
    SQLITE_EXTENSION_INIT2(api);
    return sqlite3_create_function(db, "js_eval", 1, SQLITE_UTF8, (void*)db, js_eval_func, NULL, NULL);
  }
}
