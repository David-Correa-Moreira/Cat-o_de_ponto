<?php

namespace App\Models;

use Illuminate\Database\Eloquent\Model;

class U extends Model
{
    protected $table = 'u';

    public $timestamps = false;

    protected $fillable = ['u', 's'];

    public function r()
    {
        return $this->hasMany(R::class, 'r', 'u'); //especificado relacionamento, chave estrangeira e chave local
    }

    //mutator
    public function setUAttribute($value)
    {
    $this->attributes['u'] = strtoupper($value);
    }
}
