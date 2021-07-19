<?php

namespace App\Models;

use Illuminate\Database\Eloquent\Model;

class R extends Model
{
    protected $table = 'r';

    protected $fillable = ['r'];

    public function u()
    {
        return $this->belongsTo(U::class);
    }
}
