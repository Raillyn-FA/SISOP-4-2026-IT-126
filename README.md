# SISOP-4-2026-IT-126

| Modul 4 |   Identitas Praktikan  |
|---------|------------------------|
| Nama    | Rayhan Fadhilah Allayn |
| NRP     | 5027251126             |
| Kelas   | Sistem Operasi B       |
| Asisten | MOO                    |

## Struktur Repository

![alt text](assets/StrukturRepository.png)

# Soal 1 - Save Asisten Kenz
Pada soal ini diminta membuat filesystem virtual menggunakan FUSE. Filesystem akan melakukan passthrough terhadap folder sumber dan menambahkan sebuah virtual file bernama `tujuan.txt`.

## Penjelasan

Filesystem dibuat menggunakan FUSE dengan beberapa callback utama:
- `getattr`
- `readdir`
- `open`
- `read`

File virtual `tujuan.txt` dibuat secara dinamis dengan mengambil seluruh isi koordinat (`KOORD:`) dari file `1.txt` sampai `7.txt`.

---
## Penjelasan Program

Program menggunakan FUSE untuk:
- melakukan passthrough filesystem
- membaca seluruh file `1.txt` hingga `7.txt`
- mengambil seluruh koordinat setelah string `KOORD:`
- membuat file virtual `tujuan.txt`

---

## Callback FUSE yang Digunakan

### `getattr`
Digunakan untuk mengambil metadata file.

Pada callback ini juga dilakukan pembuatan metadata virtual untuk `tujuan.txt`.

```c
if (strcmp(path, "/tujuan.txt") == 0)
```

---

### `readdir`
Digunakan untuk membaca isi direktori.

Program menambahkan file virtual:

```c
filler(buf, "tujuan.txt", &st, 0);
```

---

### `read`
Digunakan untuk membaca isi file.

Program membaca seluruh file koordinat lalu menggabungkannya menjadi isi `tujuan.txt`.

---

## Cara Menjalankan

### Compile

```bash
gcc kenz_rescue.c `pkg-config fuse --cflags --libs` -o kenz_rescue
```

### Mount

```bash
./kenz_rescue amba_files mnt
```

### Unmount

```bash
fusermount -u mnt
```

---

## Screenshot

### Struktur Folder
![Screenshot](assets/soal1/struktur.png)

### Mount Filesystem
![Screenshot](assets/soal1/mount.png)

### Isi tujuan.txt
![Screenshot](assets/soal1/tujuan.png)

---

# Soal 2

## Deskripsi
Pada soal ini dibuat encrypted filesystem menggunakan FUSE dengan metode XOR serta integrasi Docker dan TCP client-server.

---

## Struktur Folder

```bash
soal_2/
├── Dockerfile
├── client.c
├── encrypted_storage/
├── fuse.c
├── fuse_mount/
└── server
```

---

## Penjelasan Program

Filesystem akan:
- menyembunyikan extension `.enc`
- melakukan decrypt otomatis saat file dibaca
- melakukan encrypt otomatis saat file ditulis

Metode encryption menggunakan XOR.

```c
buf[i] ^= XOR_KEY;
```

---

## Callback FUSE yang Digunakan

### `readdir`
Menghilangkan extension `.enc` saat file ditampilkan.

```c
char *ext = strrchr(name, '.');
```

---

### `read`
Melakukan decrypt otomatis menggunakan XOR.

```c
xor_buffer(buf, res);
```

---

### `write`
Melakukan encrypt otomatis sebelum data ditulis.

```c
xor_buffer(tmp, size);
```

---

## TCP Client

Client menggunakan socket TCP untuk terhubung ke server pada port `9000`.

```c
server.sin_port = htons(9000);
```

---

## Docker

Container digunakan untuk menjalankan database server.

### Build Docker

```bash
docker build -t soal-2-modul-4-sisop .
```

### Run Container

```bash
docker run -d \
--name db_app \
-p 9000:9000 \
-v $(pwd)/fuse_mount:/app/db \
soal-2-modul-4-sisop
```

---

## Cara Menjalankan

### Compile FUSE

```bash
gcc fuse.c `pkg-config fuse --cflags --libs` -o securefs
```

### Mount Filesystem

```bash
./securefs -o allow_other fuse_mount
```

### Compile Client

```bash
gcc client.c -o client
```

### Jalankan Client

```bash
./client
```

---

## Screenshot

### Encrypted Storage
![Screenshot](assets/soal2/storage.png)

### FUSE Mount
![Screenshot](assets/soal2/mount.png)

### Docker Running
![Screenshot](assets/soal2/docker.png)

### Client Connected
![Screenshot](assets/soal2/client.png)

---

# Soal 3

## Deskripsi
Pada soal ini dibuat file sharing server menggunakan Samba dan Docker Compose dengan pengaturan permission berdasarkan role user.

---

## Struktur Folder

```bash
soal_3/
├── Dockerfile
├── docker-compose.yml
├── smb.conf
├── entrypoint.sh
├── data/
│   ├── docs/
│   ├── ebooks/
│   ├── papers/
│   └── sourcecode/
└── logs/
    └── libraryit.log
```

---

## Penjelasan Program

Sistem menggunakan:
- Docker Compose
- Samba Server
- Linux Permission
- Volume Mounting

Terdapat 3 user:
- member
- contributor
- librarian

---

## Konfigurasi Samba

### Sourcecode Hidden Share

```ini
browseable = no
```

---

### Docs Write Restriction

```ini
read only = yes
write list = librarian
```

---

## Permission Linux

```bash
chmod -R 750 /libraryit/sourcecode
chmod -R 770 /libraryit/docs
```

---

## Docker Compose

### Menjalankan Container

```bash
docker compose up -d --build
```

### Stop Container

```bash
docker compose down
```

---

## Testing Samba

### List Share

```bash
smbclient -L //localhost -p 1445 -U member%member123
```

### Test Forbidden Access

```bash
smbclient //localhost/sourcecode -p 1445 -U member%member123
```

---

## Screenshot

### Docker Compose Running
![Screenshot](assets/soal3/docker.png)

### Samba User
![Screenshot](assets/soal3/user.png)

### Access Denied
![Screenshot](assets/soal3/denied.png)

### Logger
![Screenshot](assets/soal3/logger.png)

---

# Kendala

## Soal 2
Mengalami masalah bind mount Docker dengan FUSE pada WSL sehingga diperlukan mount dengan `allow_other`.

---

## Soal 3
Permission Linux sempat menyebabkan `librarian` tidak dapat write ke folder docs.

---

# Kesimpulan

Pada modul ini dipelajari:
- Filesystem virtual menggunakan FUSE
- Encryption filesystem
- Docker containerization
- TCP client-server
- Samba file sharing
- Linux permission dan ownership
- Docker Compose dan volume mounting
