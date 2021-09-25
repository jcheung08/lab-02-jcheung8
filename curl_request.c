#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct url_data {
  size_t size;
  char *data;
};

size_t write_data(void *ptr, size_t size, size_t nmemb, struct url_data *data) {
  size_t index = data->size;
  size_t n = (size * nmemb);
  char *tmp;

  data->size += (size * nmemb);

#ifdef DEBUG
  fprintf(stderr, "data at %p size=%ld nmemb=%ld\n", ptr, size, nmemb);
#endif
  tmp = realloc(data->data, data->size + 1); /* +1 for '\0' */

  if (tmp) {
    data->data = tmp;
  } else {
    if (data->data) {
      free(data->data);
    }
    fprintf(stderr, "Failed to allocate memory.\n");
    return 0;
  }

  memcpy((data->data + index), ptr, n);
  data->data[data->size] = '\0';

  return size * nmemb;
}

char *request(char *type, char *base_url, char *kv) {
  CURL *curl;

  struct url_data data;
  data.size = 0;
  data.data = malloc(4096); /* reasonable size initial buffer */
  if (NULL == data.data) {
    fprintf(stderr, "Failed to allocate memory.\n");
    return NULL;
  }

  data.data[0] = '\0';

  CURLcode res;

  char url[1024];
  strcpy(url, base_url);

  if (strcmp(type, "get") == 0) {
    strcat(url, "/?");
    strcat(url, kv);
  }

  curl = curl_easy_init();
  if (curl) {
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);

    if (strcmp(type, "post") == 0)
      curl_easy_setopt(curl, CURLOPT_POSTFIELDS, kv);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));
    }

    curl_easy_cleanup(curl);
  }
  return data.data;
}

int main(int argc, char **argv) {
  char *type = argv[1];
  char *kv = argv[2];
  char *url = "http://localhost:8080";
  char *res = request(type, url, kv);
  printf("%s\n", res);
  free(res);
  return 0;
}
