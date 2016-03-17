/* Copyright (c) 2004, 2013, Oracle and/or its affiliates. All rights reserved.
   Copyright (c) 2015, yoku0825. All rights reserved.

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; version 2 of the License.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#include "sql_priv.h"
#include "sql_class.h"           // MYSQL_HANDLERTON_INTERFACE_VERSION
#include "ha_zundoko.h"
#include "probes_mysql.h"
#include "sql_plugin.h"

#define KIYOSHI "キ・ヨ・シ！"
uint pos;
int zundoko_buffer= 0;
char zundoko[2][8]= {"ズン", "ドコ"};
int next_is_eof= 0;

static handler *zundoko_create_handler(handlerton *hton,
                                     TABLE_SHARE *table, 
                                     MEM_ROOT *mem_root);

handlerton *zundoko_hton;

static const char* zundoko_system_database();
static bool zundoko_is_supported_system_table(const char *db,
                                      const char *table_name,
                                      bool is_sql_layer_system_table);
#ifdef HAVE_PSI_INTERFACE
static PSI_mutex_key ex_key_mutex_Zundoko_share_mutex;

static PSI_mutex_info all_zundoko_mutexes[]=
{
  { &ex_key_mutex_Zundoko_share_mutex, "Zundoko_share::mutex", 0}
};

static void init_zundoko_psi_keys()
{
  const char* category= "zundoko";
  int count;

  count= array_elements(all_zundoko_mutexes);
  mysql_mutex_register(category, all_zundoko_mutexes, count);
}
#endif

Zundoko_share::Zundoko_share()
{
  thr_lock_init(&lock);
  mysql_mutex_init(ex_key_mutex_Zundoko_share_mutex,
                   &mutex, MY_MUTEX_INIT_FAST);
}


static int zundoko_init_func(void *p)
{
  DBUG_ENTER("zundoko_init_func");

#ifdef HAVE_PSI_INTERFACE
  init_zundoko_psi_keys();
#endif

  zundoko_hton= (handlerton *)p;
  zundoko_hton->state=                     SHOW_OPTION_YES;
  zundoko_hton->create=                    zundoko_create_handler;
  zundoko_hton->flags=                     HTON_CAN_RECREATE;
  zundoko_hton->system_database=           zundoko_system_database;
  zundoko_hton->is_supported_system_table= zundoko_is_supported_system_table;

  DBUG_RETURN(0);
}


Zundoko_share *ha_zundoko::get_share()
{
  Zundoko_share *tmp_share;

  DBUG_ENTER("ha_zundoko::get_share()");

  lock_shared_ha_data();
  if (!(tmp_share= static_cast<Zundoko_share*>(get_ha_share_ptr())))
  {
    tmp_share= new Zundoko_share;
    if (!tmp_share)
      goto err;

    set_ha_share_ptr(static_cast<Handler_share*>(tmp_share));
  }
err:
  unlock_shared_ha_data();
  DBUG_RETURN(tmp_share);
}


static handler* zundoko_create_handler(handlerton *hton,
                                     TABLE_SHARE *table, 
                                     MEM_ROOT *mem_root)
{
  return new (mem_root) ha_zundoko(hton, table);
}

ha_zundoko::ha_zundoko(handlerton *hton, TABLE_SHARE *table_arg)
  :handler(hton, table_arg)
{}


static const char *ha_zundoko_exts[] = {
  NullS
};

const char **ha_zundoko::bas_ext() const
{
  return ha_zundoko_exts;
}

const char* ha_zundoko_system_database= NULL;
const char* zundoko_system_database()
{
  return ha_zundoko_system_database;
}

static st_system_tablename ha_zundoko_system_tables[]= {
  {(const char*)NULL, (const char*)NULL}
};

static bool zundoko_is_supported_system_table(const char *db,
                                              const char *table_name,
                                              bool is_sql_layer_system_table)
{
  st_system_tablename *systab;

  // Does this SE support "ALL" SQL layer system tables ?
  if (is_sql_layer_system_table)
    return false;

  // Check if this is SE layer system tables
  systab= ha_zundoko_system_tables;
  while (systab && systab->db)
  {
    if (systab->db == db &&
        strcmp(systab->tablename, table_name) == 0)
      return true;
    systab++;
  }

  return false;
}


int ha_zundoko::open(const char *name, int mode, uint test_if_locked)
{
  DBUG_ENTER("ha_zundoko::open");

  if (!(share = get_share()))
    DBUG_RETURN(1);
  thr_lock_data_init(&share->lock,&lock,NULL);

  DBUG_RETURN(0);
}


int ha_zundoko::close(void)
{
  DBUG_ENTER("ha_zundoko::close");
  DBUG_RETURN(0);
}


int ha_zundoko::write_row(uchar *buf)
{
  DBUG_ENTER("ha_zundoko::write_row");
  DBUG_RETURN(HA_ERR_WRONG_COMMAND);
}


int ha_zundoko::update_row(const uchar *old_data, uchar *new_data)
{

  DBUG_ENTER("ha_zundoko::update_row");
  DBUG_RETURN(HA_ERR_WRONG_COMMAND);
}


int ha_zundoko::delete_row(const uchar *buf)
{
  DBUG_ENTER("ha_zundoko::delete_row");
  DBUG_RETURN(HA_ERR_WRONG_COMMAND);
}


int ha_zundoko::index_read_map(uchar *buf, const uchar *key,
                               key_part_map keypart_map __attribute__((unused)),
                               enum ha_rkey_function find_flag
                               __attribute__((unused)))
{
  int rc;
  DBUG_ENTER("ha_zundoko::index_read");
  MYSQL_INDEX_READ_ROW_START(table_share->db.str, table_share->table_name.str);
  rc= HA_ERR_WRONG_COMMAND;
  MYSQL_INDEX_READ_ROW_DONE(rc);
  DBUG_RETURN(rc);
}


int ha_zundoko::index_next(uchar *buf)
{
  int rc;
  DBUG_ENTER("ha_zundoko::index_next");
  MYSQL_INDEX_READ_ROW_START(table_share->db.str, table_share->table_name.str);
  rc= HA_ERR_WRONG_COMMAND;
  MYSQL_INDEX_READ_ROW_DONE(rc);
  DBUG_RETURN(rc);
}


int ha_zundoko::index_prev(uchar *buf)
{
  int rc;
  DBUG_ENTER("ha_zundoko::index_prev");
  MYSQL_INDEX_READ_ROW_START(table_share->db.str, table_share->table_name.str);
  rc= HA_ERR_WRONG_COMMAND;
  MYSQL_INDEX_READ_ROW_DONE(rc);
  DBUG_RETURN(rc);
}


int ha_zundoko::index_first(uchar *buf)
{
  int rc;
  DBUG_ENTER("ha_zundoko::index_first");
  MYSQL_INDEX_READ_ROW_START(table_share->db.str, table_share->table_name.str);
  rc= HA_ERR_WRONG_COMMAND;
  MYSQL_INDEX_READ_ROW_DONE(rc);
  DBUG_RETURN(rc);
}


int ha_zundoko::index_last(uchar *buf)
{
  int rc;
  DBUG_ENTER("ha_zundoko::index_last");
  MYSQL_INDEX_READ_ROW_START(table_share->db.str, table_share->table_name.str);
  rc= HA_ERR_WRONG_COMMAND;
  MYSQL_INDEX_READ_ROW_DONE(rc);
  DBUG_RETURN(rc);
}


int ha_zundoko::rnd_init(bool scan)
{
  DBUG_ENTER("ha_zundoko::rnd_init");
  pos= 0;
  DBUG_RETURN(0);
}

int ha_zundoko::rnd_end()
{
  DBUG_ENTER("ha_zundoko::rnd_end");
  DBUG_RETURN(0);
}


int ha_zundoko::rnd_next(uchar *buf)
{
  int rc;
  DBUG_ENTER("ha_zundoko::rnd_next");
  MYSQL_READ_ROW_START(table_share->db.str, table_share->table_name.str,
                       TRUE);
  rc= 0;

  if (next_is_eof)
  {
    next_is_eof= 0;
    rc= HA_ERR_END_OF_FILE;
    goto end;
  }

  if (zundoko_buffer == 62) // 11110
  {
    for (Field **field= table->field; *field; field++)
    {
      (*field)->store(KIYOSHI, strlen(KIYOSHI), system_charset_info);
      (*field)->set_notnull();
    }
    pos++;
    zundoko_buffer= 0;
    next_is_eof= 1;
  }
  else
  {
    zundoko_buffer= zundoko_buffer << 1;
    zundoko_buffer= zundoko_buffer & 63; //truncate over 6th bit.

    for (Field **field= table->field; *field; field++)
    {
      int rnd= rand() % 2;

      zundoko_buffer += rnd;

      (*field)->store(zundoko[rnd], strlen(zundoko[rnd]), system_charset_info);
      (*field)->set_notnull();
      pos++;
    }
  }


end:
  MYSQL_READ_ROW_DONE(rc);
  DBUG_RETURN(rc);
}


void ha_zundoko::position(const uchar *record)
{
  DBUG_ENTER("ha_zundoko::position");
  DBUG_VOID_RETURN;
}


int ha_zundoko::rnd_pos(uchar *buf, uchar *pos)
{
  int rc;
  DBUG_ENTER("ha_zundoko::rnd_pos");
  MYSQL_READ_ROW_START(table_share->db.str, table_share->table_name.str,
                       TRUE);
  rc= HA_ERR_WRONG_COMMAND;
  MYSQL_READ_ROW_DONE(rc);
  DBUG_RETURN(rc);
}


int ha_zundoko::info(uint flag)
{
  DBUG_ENTER("ha_zundoko::info");
  DBUG_RETURN(0);
}


int ha_zundoko::extra(enum ha_extra_function operation)
{
  DBUG_ENTER("ha_zundoko::extra");
  DBUG_RETURN(0);
}


int ha_zundoko::delete_all_rows()
{
  DBUG_ENTER("ha_zundoko::delete_all_rows");
  DBUG_RETURN(HA_ERR_WRONG_COMMAND);
}


int ha_zundoko::truncate()
{
  DBUG_ENTER("ha_zundoko::truncate");
  DBUG_RETURN(HA_ERR_WRONG_COMMAND);
}


int ha_zundoko::external_lock(THD *thd, int lock_type)
{
  DBUG_ENTER("ha_zundoko::external_lock");
  DBUG_RETURN(0);
}


THR_LOCK_DATA **ha_zundoko::store_lock(THD *thd,
                                       THR_LOCK_DATA **to,
                                       enum thr_lock_type lock_type)
{
  if (lock_type != TL_IGNORE && lock.type == TL_UNLOCK)
    lock.type=lock_type;
  *to++= &lock;
  return to;
}


int ha_zundoko::delete_table(const char *name)
{
  DBUG_ENTER("ha_zundoko::delete_table");
  /* This is not implemented but we want someone to be able that it works. */
  DBUG_RETURN(0);
}


int ha_zundoko::rename_table(const char * from, const char * to)
{
  DBUG_ENTER("ha_zundoko::rename_table ");
  DBUG_RETURN(HA_ERR_WRONG_COMMAND);
}


ha_rows ha_zundoko::records_in_range(uint inx, key_range *min_key,
                                     key_range *max_key)
{
  DBUG_ENTER("ha_zundoko::records_in_range");
  DBUG_RETURN(10);                         // low number to force index usage
}


int ha_zundoko::create(const char *name, TABLE *table_arg,
                       HA_CREATE_INFO *create_info)
{
  DBUG_ENTER("ha_zundoko::create");
  /*
    This is not implemented but we want someone to be able to see that it
    works.
  */
  DBUG_RETURN(0);
}


struct st_mysql_storage_engine zundoko_storage_engine=
{ MYSQL_HANDLERTON_INTERFACE_VERSION };

static struct st_mysql_sys_var* zundoko_system_variables[]= {
  NULL
};

static struct st_mysql_show_var func_status[]=
{
  {0,0,SHOW_UNDEF}
};

mysql_declare_plugin(zundoko)
{
  MYSQL_STORAGE_ENGINE_PLUGIN,
  &zundoko_storage_engine,
  "zundoko",
  "yoku0825",
  "zundoko storage engine for MySQL",
  PLUGIN_LICENSE_GPL,
  zundoko_init_func,                            /* Plugin Init */
  NULL,                                         /* Plugin Deinit */
  0x0001 /* 0.1 */,
  func_status,                                  /* status variables */
  zundoko_system_variables,                     /* system variables */
  NULL,                                         /* config options */
  0,                                            /* flags */
}
mysql_declare_plugin_end;
